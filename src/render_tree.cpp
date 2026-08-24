#include "render_tree.hpp"
#include "hash_combine.hpp"
#include "new_arch.hpp"
#include "new_sizing.hpp"
#include "overloaded.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <chrono>
#include <print>
#include <variant>

namespace tree {
    using layout::FlexLayout;
    using layout::FlexResolver;
    using layout::GridResolver;
    using layout::LayoutOutput;
    using layout::MarginMetadata;
    using layout::Measured;
    using layout::SizeResolutionContext;
    using layout::Axis;
    using layout::AxisResolution;
    using layout::IntrinsicSizes;
    using style::ClipUniform;
    using style::SizeError;
    using style::Unit;

    void RenderTree::markDirty(std::source_location source) {
        needsUpdate = true;
        pendingFrameBufferWrites = MaxOutstandingFrameCount;
        renderOrderDirty = true;
        instrumentation::recordRenderOrderInvalidation(
            std::to_underlying(instrumentation::RenderOrderReason::FullTreeDirty)
        );
        if (auto root = getRoot()) {
            instrumentation::recordMutation(
                root->id,
                std::to_underlying(allPhaseDirtyBits()),
                std::to_underlying(allPhaseDirtyBits()),
                root->children.empty()
                    ? instrumentation::DirtyPropagation::None
                    : instrumentation::DirtyPropagation::Descendants,
                source
            );
            markSubtreeDirty(root, allPhaseDirtyBits());
        }
    }

    bool RenderTree::isFrameInfoChanged(const FrameInfo& frameInfo) const {
      return !lastFrameInfo.has_value()
          || lastFrameInfo->width != frameInfo.width
          || lastFrameInfo->height != frameInfo.height
          || lastFrameInfo->scale != frameInfo.scale;
  }

  bool RenderTree::requiresFrame(const FrameInfo& frameInfo) const {
      uint32_t reasons = 0;

      if (needsUpdate) {
          reasons |= std::to_underlying(
              instrumentation::FrameReason::Mutation
          );
      }
      if (isFrameInfoChanged(frameInfo)) {
          reasons |= std::to_underlying(
              instrumentation::FrameReason::FrameInfoChanged
          );
      }
      if (pendingFrameBufferWrites > 0) {
          reasons |= std::to_underlying(
              instrumentation::FrameReason::PendingBufferWrites
          );
      }

      instrumentation::recordFrameDecision(reasons);
      return reasons != 0;
  }

    void RenderTree::markDirty(TreeNode* node, DirtyBits bits, std::source_location source) {
        if (!node || bits == DirtyBits::None) return;

        needsUpdate = true;
        pendingFrameBufferWrites = MaxOutstandingFrameCount;

        if (hasDirty(bits, DirtyBits::PaintOrder)) {
            renderOrderDirty = true;
            instrumentation::recordRenderOrderInvalidation(std::to_underlying(
                instrumentation::RenderOrderReason::PaintOrderChanged
            ));
        }

        DirtyBits selfBits = bits;
        if (hasDirty(bits, DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout)) {
            selfBits |= DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }
        if (hasDirty(bits, DirtyBits::PostLayout)) {
            selfBits |= DirtyBits::Place | DirtyBits::Finalize;
        }
        if (hasDirty(bits, DirtyBits::Place)) {
            selfBits |= DirtyBits::Finalize;
        }

        uint8_t propagation = 0;
        if (node->parent) {
            propagation |= std::to_underlying(instrumentation::DirtyPropagation::Ancestors);
        }
        if (!node->children.empty() && hasDirty(bits, DirtyBits::PostLayout | DirtyBits::Place)) {
            propagation |= std::to_underlying(instrumentation::DirtyPropagation::Descendants);
        }

        instrumentation::recordMutation(
            node->id,
            std::to_underlying(bits),
            std::to_underlying(selfBits),
            static_cast<instrumentation::DirtyPropagation>(propagation),
            source
        );

        node->dirtySelf |= selfBits;
        node->dirtySubtree |= selfBits;

        for (auto* ancestor = node->parent; ancestor; ancestor = ancestor->parent) {
            ancestor->dirtySubtree |= selfBits;
        }

        if (hasDirty(bits, DirtyBits::PostLayout | DirtyBits::Place)) {
            markSubtreeDirty(node, selfBits & (DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize));
        }

        if (hasDirty(bits, DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout)) {
            DirtyBits ancestorBits = DirtyBits::Layout | DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
            for (auto* ancestor = node->parent; ancestor; ancestor = ancestor->parent) {
                ancestor->dirtySelf |= ancestorBits;
                ancestor->dirtySubtree |= ancestorBits | selfBits;
            }
        }

        if (hasDirty(bits, DirtyBits::PostLayout)) {
            DirtyBits ancestorBits = DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
            for (auto* ancestor = node->parent; ancestor; ancestor = ancestor->parent) {
                ancestor->dirtySelf |= ancestorBits;
                ancestor->dirtySubtree |= ancestorBits | selfBits;
            }
        }
    }

    void RenderTree::markSubtreeDirty(TreeNode* node, DirtyBits bits) {
        if (!node || bits == DirtyBits::None) return;
        node->dirtySelf |= bits;
        node->dirtySubtree |= bits;
        for (auto& child : node->children) {
            markSubtreeDirty(child.get(), bits);
        }
    }

    void RenderTree::clearDirty(TreeNode* node) {
        if (!node) return;
        node->dirtySelf = DirtyBits::None;
        node->dirtySubtree = DirtyBits::None;
        for (auto& child : node->children) {
            clearDirty(child.get());
        }
    }

    bool RenderTree::subtreeHasDirty(TreeNode* node, DirtyBits bits) const {
        if (!node) return false;
        return hasDirty(node->dirtySelf | node->dirtySubtree, bits);
    }

    ConstraintsKey RenderTree::makeConstraintsKey(const Constraints& constraints,
                                                  simd_float2 extraOriginA,
                                                  simd_float2 extraOriginB) const {
        std::size_t hash = 0;
        hash_combine(hash, constraints.origin.x);
        hash_combine(hash, constraints.origin.y);
        hash_combine(hash, constraints.cursor.x);
        hash_combine(hash, constraints.cursor.y);
        hash_combine(hash, constraints.availableWidth.value);
        hash_combine(hash, static_cast<int>(constraints.availableWidth.unit));
        hash_combine(hash, constraints.availableHeight.value);
        hash_combine(hash, static_cast<int>(constraints.availableHeight.unit));
        hash_combine(hash, static_cast<int>(constraints.inheritedProperties.direction));
        hash_combine(hash, static_cast<int>(constraints.inheritedProperties.textAlign));
        hash_combine(hash, constraints.frameInfo.width);
        hash_combine(hash, constraints.frameInfo.height);
        hash_combine(hash, constraints.frameInfo.scale);
        hash_combine(hash, constraints.absoluteContainingBlock.origin.x);
        hash_combine(hash, constraints.absoluteContainingBlock.origin.y);
        hash_combine(hash, constraints.absoluteContainingBlock.width.value);
        hash_combine(hash, static_cast<int>(constraints.absoluteContainingBlock.width.unit));
        hash_combine(hash, constraints.absoluteContainingBlock.height.value);
        hash_combine(hash, static_cast<int>(constraints.absoluteContainingBlock.height.unit));
        hash_combine(hash, static_cast<int>(constraints.edgeIntent.edgeDisplayMode));
        hash_combine(hash, constraints.edgeIntent.intent);
        hash_combine(hash, constraints.edgeIntent.collapsable);
        hash_combine(hash, constraints.prevInlineHeight);
        auto hashOptionalSize = [&](const std::optional<Size>& size) {
            hash_combine(hash, size.has_value());
            if (size.has_value()) {
                hash_combine(hash, size->value);
                hash_combine(hash, static_cast<int>(size->unit));
            }
        };
        hashOptionalSize(constraints.replacedAttributes.marginTop);
        hashOptionalSize(constraints.replacedAttributes.marginBottom);
        hash_combine(hash, constraints.shrinkWidthToFit);
        hash_combine(hash, constraints.shrinkHeightToFit);
        hash_combine(hash, constraints.widthResolution.has_value());
        if (constraints.widthResolution.has_value()) hash_combine(hash, static_cast<int>(*constraints.widthResolution));
        hash_combine(hash, constraints.heightResolution.has_value());
        if (constraints.heightResolution.has_value()) hash_combine(hash, static_cast<int>(*constraints.heightResolution));
        hash_combine(hash, constraints.intrinsicSizesAxis.has_value());
        if (constraints.intrinsicSizesAxis.has_value()) hash_combine(hash, static_cast<int>(*constraints.intrinsicSizesAxis));
        auto lineFragments = constraints.inlineFormatting.lineFragments();
        auto lineBoxes = constraints.inlineFormatting.lineBoxes();
        hash_combine(hash, lineFragments.size());
        hash_combine(hash, lineBoxes.size());
        hash_combine(hash, constraints.textBidiInput.has_value());
        if (constraints.textBidiInput.has_value()) {
            const auto& bidi = *constraints.textBidiInput;
            hash_combine(hash, bidi.paragraphByteStart);
            hash_combine(hash, bidi.byteLength);
            for (const auto& run : bidi.runs) {
                hash_combine(hash, run.byteStart);
                hash_combine(hash, run.byteLength);
                hash_combine(hash, run.level);
            }
        }
        hash_combine(hash, constraints.textOverflow.has_value());
        if (constraints.textOverflow.has_value()) {
            hash_combine(hash, static_cast<int>(constraints.textOverflow->mode));
            for (unsigned char byte : constraints.textOverflow->ending) {
                hash_combine(hash, byte);
            }
        }

        for (auto& clip : constraints.clipUniforms) {
            hash_combine(hash, clip.rectCenter.x);
            hash_combine(hash, clip.rectCenter.y);
            hash_combine(hash, clip.halfExtent.x);
            hash_combine(hash, clip.halfExtent.y);
            hash_combine(hash, clip.cornerRadius.x);
            hash_combine(hash, clip.cornerRadius.y);
        }

        for (auto& fragment : lineFragments) {
            hash_combine(hash, fragment.width);
            hash_combine(hash, fragment.atomStart);
            hash_combine(hash, fragment.atomCount);
            hash_combine(hash, fragment.lineBoxIndex);
            hash_combine(hash, fragment.fragmentIndex);
        }
        for (auto& lineBox : lineBoxes) {
            hash_combine(hash, lineBox.fragmentCount);
            hash_combine(hash, lineBox.width);
            hash_combine(hash, lineBox.currentFragmentOffset);
            for (auto offset : lineBox.fragmentOffsets) {
                hash_combine(hash, offset);
            }
        }

        hash_combine(hash, extraOriginA.x);
        hash_combine(hash, extraOriginA.y);
        hash_combine(hash, extraOriginB.x);
        hash_combine(hash, extraOriginB.y);

        return ConstraintsKey{.value = hash};
    }

    ConstraintsKey RenderTree::makeSpeculativeKey(
        const TreeNode* node,
        const Constraints& constraints,
        const Measured& measured
    ) const {
        auto key = makeConstraintsKey(constraints);
        hash_combine(key.value, node->id);
        hash_combine(key.value, measured.explicitWidth.has_value());
        if (measured.explicitWidth.has_value()) {
            hash_combine(key.value, *measured.explicitWidth);
        } else {
            hash_combine(key.value, static_cast<int>(measured.explicitWidth.error()));
        }
        hash_combine(key.value, measured.explicitHeight.has_value());
        if (measured.explicitHeight.has_value()) {
            hash_combine(key.value, *measured.explicitHeight);
        } else {
            hash_combine(key.value, static_cast<int>(measured.explicitHeight.error()));
        }
        return key;
    }

    instrumentation::RecomputeReason RenderTree::recomputeReason(
        TreeNode* node,
        DirtyBits bit,
        const ConstraintsKey& incomingKey
    ) const {
        using instrumentation::RecomputeReason;

        if (hasDirty(node->dirtySelf, bit)) return RecomputeReason::Dirty;
        if (!node->constraintsKey.has_value()) return RecomputeReason::MissingConstraintsKey;
        if (*node->constraintsKey != incomingKey) return RecomputeReason::ConstraintsChanged;
        return RecomputeReason::None;
    }

    const std::vector<TreeNode*>& RenderTree::sortedRenderOrder() {
        if (!renderOrderDirty && !renderOrderCache.empty()) {
            instrumentation::recordRenderOrderCache(true);
            return renderOrderCache;
        }

        auto rebuildStartedAt = std::chrono::steady_clock::time_point{};
        if constexpr (instrumentation::enabled) {
            rebuildStartedAt = std::chrono::steady_clock::now();
        }
        uint32_t immediateReason = renderOrderCache.empty()
            ? std::to_underlying(instrumentation::RenderOrderReason::EmptyCache)
            : 0;

        renderOrderCache = collectAllNodes(getRoot());

        uint64_t paintOrderIndex = 0;

        std::function<void(TreeNode*)> assignPaintOrderIndices = [&](TreeNode* node) {
            if (!node) return;

            node->paintPreorderIndex = paintOrderIndex++;
            for (auto& child : node->children) {
                assignPaintOrderIndices(child.get());
            }
            node->paintPostorderIndex = paintOrderIndex++;
        };

        assignPaintOrderIndices(getRoot());

        std::sort(renderOrderCache.begin(), renderOrderCache.end(), [](TreeNode* a, TreeNode* b) {
            if (a->globalZIndex != b->globalZIndex) {
                return a->globalZIndex < b->globalZIndex;
            }

            auto aIsAncestor = a->paintPreorderIndex < b->paintPreorderIndex
                && b->paintPostorderIndex < a->paintPostorderIndex;
            auto bIsAncestor = b->paintPreorderIndex < a->paintPreorderIndex
                && a->paintPostorderIndex < b->paintPostorderIndex;

            if (aIsAncestor) {
                return true;
            }
            if (bIsAncestor) {
                return false;
            }

            // return a->id < b->id;

            return a->paintPreorderIndex < b->paintPreorderIndex;
        });

        renderOrderDirty = false;
        if constexpr (instrumentation::enabled) {
            instrumentation::recordRenderOrderCache(
                false,
                immediateReason,
                std::chrono::steady_clock::now() - rebuildStartedAt
            );
        }
        return renderOrderCache;
    }

    // I have a render cache, develop some sort of caching policy that makes these useful
    void RenderTree::update(const FrameInfo& frameInfo, uint64_t frameIndex) {
        bool frameInfoChanged = isFrameInfoChanged(frameInfo);
        if (frameInfoChanged) {
            pendingFrameBufferWrites = MaxOutstandingFrameCount;
            if (auto root = getRoot()) {
                markSubtreeDirty(root, allPhaseDirtyBits());
            }
            renderOrderDirty = true;
            instrumentation::recordRenderOrderInvalidation(
                std::to_underlying(instrumentation::RenderOrderReason::FrameInfoChanged)
            );
        }

        if (!needsUpdate && !frameInfoChanged && pendingFrameBufferWrites == 0) {
            return;
        }

        needsUpdate = false;
        lastFrameInfo = frameInfo;

        auto root = getRoot();
        if (!root) return;


        rootCursor = simd_float2{0,0};
        rootConstraints = Constraints{
            .origin = simd_float2{0,0},
            .cursor = rootCursor,
            .availableWidth = Size::px(frameInfo.width),
            .availableHeight = Size::px(frameInfo.height),
            .frameInfo = frameInfo,
            .absoluteContainingBlock = {
                .origin = {0, 0},
                .width = Size::px(frameInfo.width),
                .height = Size::px(frameInfo.height)
            },
            .clipUniforms = {
                ClipUniform {
                    .rectCenter = {frameInfo.width * 0.5f, frameInfo.height * 0.5f},
                    .halfExtent = {frameInfo.width * 0.5f, frameInfo.height * 0.5f},
                    .cornerRadius = {0.0f, 0.0f}
                }
            },
        };

        // AHH APPLE CLANG DOESN'T SUPPORT EXECUTION POLICIES YET EXECUTE ME
        // Parallel::for_each(allNodes.begin(), allNodes.end(),
        //     [&](TreeNode* node) {
        //         node->measured = node->element->measure(rootConstraints);
        //     }
        // );

        if (subtreeHasDirty(root, DirtyBits::Measure) || !root->measured.has_value()) {
            instrumentation::PhaseTimer timer{instrumentation::Phase::Measure};
            measurePhase(root, rootConstraints);
        }
        if (subtreeHasDirty(root, DirtyBits::Atomize) || !root->atomized.has_value()) {
            instrumentation::PhaseTimer timer{instrumentation::Phase::Atomize};
            if (!atomizePhase(root, rootConstraints)) return;
        }
    
        // Layout pass
        // precompute margin metadata + intents
        bool needsLayoutPass = subtreeHasDirty(root, DirtyBits::Layout) || !root->layout.has_value();
        if (needsLayoutPass && (subtreeHasDirty(root, DirtyBits::Layout) || !root->preLayout.has_value())) {
            instrumentation::PhaseTimer timer{instrumentation::Phase::PreLayout};
            preLayoutPhase(root, frameInfo, rootConstraints);
        }
        // initial layout pass
        if (needsLayoutPass) {
            speculativeLayoutCache.clear();
            instrumentation::PhaseTimer timer{instrumentation::Phase::Layout};
            layoutPhase(root, frameInfo, rootConstraints, *root->measured);
            root->calculateGlobalZIndex(0);
        }
        sortedRenderOrder();
        // postLayout: resolve global positions (serial, top-down) + reconcile atoms
        if (subtreeHasDirty(root, DirtyBits::PostLayout) || !root->layout.has_value()) {
            instrumentation::PhaseTimer timer{instrumentation::Phase::PostLayout};
            postLayoutPhase(root, frameInfo, rootConstraints, {0.0f, 0.0f}, {0.0f, 0.0f});
        }

        if (subtreeHasDirty(root, DirtyBits::Place) || !root->placed.has_value()) {
            instrumentation::PhaseTimer timer{instrumentation::Phase::Place};
            placePhase(root, frameInfo, rootConstraints);
        }
        if (subtreeHasDirty(root, DirtyBits::Finalize) || !root->finalized.has_value()) {
            instrumentation::PhaseTimer timer{instrumentation::Phase::Finalize};
            finalizePhase(root, rootConstraints);
        }

        if (pendingFrameBufferWrites > 0) {
            pendingFrameBufferWrites--;
        }
        if (pendingFrameBufferWrites == 0) {
            clearDirty(root);
        }

    }

    void RenderTree::render(MTL::RenderCommandEncoder* encoder) {
        auto& allNodes = sortedRenderOrder();
        uint64_t atomCount = 0;
        
        // serially encoded; encoders are not thread safe
        for (auto node : allNodes) {
            if (node->atomized.has_value()) {
                const auto& atomized = *node->atomized;
                atomCount += atomized.usesDrawableAtoms
                    ? atomized.drawableAtoms.size()
                    : atomized.atoms.size();
            }
            auto& finalized = node->finalized;
            node->element->encode(encoder, finalized);
        }
        instrumentation::recordRenderWork(allNodes.size(), allNodes.size(), atomCount);
    }

    void RenderTree::measurePhase(TreeNode* node, Constraints& constraints) {
        auto key = makeConstraintsKey(constraints);
        auto reason = recomputeReason(node, DirtyBits::Measure, key);
        if (reason != instrumentation::RecomputeReason::None) {
            instrumentation::recordRecompute(node->id, instrumentation::Phase::Measure, reason);
            auto measured = node->element->measure(constraints, node->shared);
            node->measured = measured;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::Atomize | DirtyBits::Layout | DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }
        
        float paddingLeft = node->shared.paddingLeft.value_or(Size{}).resolveOr(constraints.availableWidth);
        float paddingTop = node->shared.paddingTop.value_or(Size{}).resolveOr(constraints.availableHeight);
        float paddingRight = node->shared.paddingRight.value_or(Size{}).resolveOr(constraints.availableWidth);
        float paddingBottom = node->shared.paddingBottom.value_or(Size{}).resolveOr(constraints.availableHeight);

        Constraints childConstraints {};

        // std::println("padidngLeft: {} paddingRight: {}", paddingLeft, paddingRight);
    
        childConstraints.availableWidth = node->measured->explicitWidth
            ? Size::px(*node->measured->explicitWidth - paddingLeft - paddingRight)
            : Size::autoSize();
        childConstraints.availableHeight = node->measured->explicitHeight
            ? Size::px(*node->measured->explicitHeight - paddingTop - paddingBottom)
            : Size::autoSize();
        
        // std::println("maxWidth: {}", childConstraints.availableWidth);

        for (auto& child : node->children) {
            measurePhase(child.get(), childConstraints);
        }
    }


    // consider safer way of accessing cache?
    Result<void> RenderTree::atomizePhase(
        TreeNode* node,
        Constraints& constraints
    ) {
        node->textBidiInput = constraints.textBidiInput;

        auto key = makeConstraintsKey(constraints);
        auto reason = recomputeReason(node, DirtyBits::Atomize, key);
        if (reason != instrumentation::RecomputeReason::None) {
            instrumentation::recordRecompute(node->id, instrumentation::Phase::Atomize, reason);
            auto& measured  = *node->measured;
            auto& shared = node->shared;
            auto atomized = node->element->atomize(constraints, shared, measured);
            node->atomized = atomized;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::Layout | DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }

        Constraints childConstraints = constraints;
        auto childBidiInputs = prepareChildBidiInputs(
            node,
            childConstraints.inheritedProperties.direction
        );
        if (!childBidiInputs) return std::unexpected{childBidiInputs.error()};
        for (size_t i = 0; i < node->children.size(); ++i) {
            childConstraints.textBidiInput = std::move((*childBidiInputs)[i]);
            auto result = atomizePhase(node->children[i].get(), childConstraints);
            if (!result) return result;
        }
        return {};
    }

    void resolveComputedDisplays(TreeNode* node)
    {
        node->computedDisplay = node->shared.display;
        if (node->parent && node->parent->getDisplay() == Display::Flex && node->element->isReplaced() && node->shared.display == Display::Inline) {
            node->computedDisplay = Display::Block;
        }

        for (auto& child : node->children) {
            resolveComputedDisplays(child.get());
        }
    }

    void buildCollapsedChains(
        TreeNode* node,
        std::unordered_map<ChainID, CollapsedChain>& chainMap,
        ChainID& nextChainId,
        CollapsedChain* collapsedTopChain,
        CollapsedChain* collapsedBottomChain
    )
    {
        // find first/last collapsable child
        TreeNode* firstInFlowCollapsableChild = nullptr;
        TreeNode* lastInFlowCollapsableChild = nullptr;

        for (auto& child : node->children) {
            auto position = child->getPosition();
            auto display = child->getDisplay();

            if ((position == Position::Static || position == Position::Relative) && display == Display::Block) {
                if (firstInFlowCollapsableChild == nullptr) firstInFlowCollapsableChild = child.get();
                lastInFlowCollapsableChild = child.get();
            }
        }

        // check if out of flow
        auto currPosition = node->getPosition();
        bool currOutOfFlow = currPosition == Position::Absolute || currPosition == Position::Fixed;

        if (currOutOfFlow) {
            firstInFlowCollapsableChild = nullptr;
            lastInFlowCollapsableChild = nullptr;
        }

        // skip continuation if padding defined
        bool nodeBlocksTopChain = node->getPaddingTop().has_value();
        bool nodeBlocksBottomChain = node->getPaddingBottom().has_value();

        if (nodeBlocksTopChain) {
            firstInFlowCollapsableChild = nullptr;
        }

        if (nodeBlocksBottomChain) {
            lastInFlowCollapsableChild = nullptr;
        }

        Size marginTop = node->getMarginTop();
        Size marginBottom = node->getMarginBottom();

        // check if has top chains; if not, create new ones (which will be propagated to first and last child)
        CollapsedChain newCollapsedTopChain;
        CollapsedChain newCollapsedBottomChain;
        CollapsedChain* propagatedTopChain = collapsedTopChain;
        CollapsedChain* propagatedBottomChain = collapsedBottomChain;

        if (!collapsedTopChain) {
            newCollapsedTopChain = {
                .id = nextChainId++,
                .root = node,
                .intent = marginTop,
                .depth = 1
            };

            propagatedTopChain = &newCollapsedTopChain;
        }else {
            if (marginTop.value > collapsedTopChain->intent.value) {
                collapsedTopChain->intent = marginTop;
            }
            
            collapsedTopChain->depth++;

            if (!firstInFlowCollapsableChild) {
                chainMap[collapsedTopChain->id] = *collapsedTopChain;
            }
        }

        if (!collapsedBottomChain) {
            newCollapsedBottomChain = {
                .id = nextChainId++,
                .root = node,
                .intent = marginBottom,
                .depth = 1
            };

            propagatedBottomChain = &newCollapsedBottomChain;
        }else {
            if (marginBottom.value > collapsedBottomChain->intent.value) {
                collapsedBottomChain->intent = marginBottom;
            }

            collapsedBottomChain->depth++;

            if (!lastInFlowCollapsableChild) {
                chainMap[collapsedBottomChain->id] = *collapsedBottomChain;
            }
        }

        for (auto& child : node->children) {
            auto rawChild = child.get();

            if (rawChild == firstInFlowCollapsableChild) {
                buildCollapsedChains(rawChild, chainMap, nextChainId, propagatedTopChain, nullptr);
            }else if (rawChild == lastInFlowCollapsableChild) {
                buildCollapsedChains(rawChild, chainMap, nextChainId, nullptr, propagatedBottomChain);
            }else {
                buildCollapsedChains(rawChild, chainMap, nextChainId, nullptr, nullptr);
            }
        }

        MarginMetadata marginMetadata {
            .topChainId = propagatedTopChain->id,
            .bottomChainId = propagatedBottomChain->id
        };

        node->preLayout = PreLayoutResult{};

        node->preLayout->marginMetadata = marginMetadata;
    }


    void RenderTree::preLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        collapsedChainMap.clear();
        nextChainId = 0;

        resolveComputedDisplays(node);
        buildCollapsedChains(node, collapsedChainMap, nextChainId, nullptr, nullptr);

        precomputeMargins(node, constraints, collapsedChainMap);
    }

    LayoutOutput RenderTree::layoutPhase(
        TreeNode* node,
        const FrameInfo& frameInfo,
        Constraints constraints,
        Measured measured
    ) {
        return layoutRecursive(node, frameInfo, std::move(constraints), measured, true);
    }

    const LayoutOutput& RenderTree::speculateLayout(
        const FrameInfo& frameInfo,
        TreeNode* node,
        Constraints constraints,
        Measured measured
    ) {
        auto key = makeSpeculativeKey(node, constraints, measured);
        if (auto found = speculativeLayoutCache.find(key);
            found != speculativeLayoutCache.end()) {
            instrumentation::recordSpeculativeLayoutCache(true);
            return found->second;
        }

        instrumentation::recordSpeculativeLayoutCache(false);

        auto output = layoutRecursive(node, frameInfo, std::move(constraints), measured, false);
        auto [inserted, _] = speculativeLayoutCache.emplace(key, std::move(output));
        return inserted->second;
    }

    std::optional<IntrinsicSizes> RenderTree::measureIntrinsicSizes(TreeNode* node, const FrameInfo& frameInfo, Constraints constraints, Measured measured, SizeRequest sizeRequest) {
        LayoutOutput output = layoutRecursive(node, frameInfo, constraints, measured, false, std::move(sizeRequest));
        return std::move(output.intrinsicSizes);
    }

    LayoutOutput RenderTree::layoutRecursive(
        TreeNode* node,
        const FrameInfo& frameInfo,
        Constraints constraints,
        Measured measured,
        bool mutate,
        std::optional<SizeRequest> intrinsicSizeRequest
    ) {
        // struct Constraints {
        //     simd_float2 origin{};
        //     simd_float2 cursor{};
        //     Size availableWidth{Size::autoSize()};
        //     Size availableHeight{Size::autoSize()};

        //     InheritedProperties inheritedProperties{};

        //     FrameInfo frameInfo{}; // viewport size (for fixed)
        //     ContainingBlock absoluteContainingBlock{}; // for absolute: nearest positioned ancestor

        //     EdgeIntent edgeIntent{};

        //     InlineFormattingInput inlineFormatting {};
        //     std::optional<bidi::TextBidiInput> textBidiInput;

        //     ReplacedAttributes replacedAttributes {};
        //     ResolvedMargins resolvedMargins {};
        //     float prevInlineHeight{};
        //     std::vector<ClipUniform> clipUniforms {};
        //     std::optional<TextOverflow> textOverflow{};

        //     SizePair parentOverride;

        //     bool shrinkWidthToFit{false};
        //     bool shrinkHeightToFit{false};
        //     std::optional<AxisResolution> widthResolution;
        //     std::optional<AxisResolution> heightResolution;
        //     std::optional<Axis> intrinsicSizesAxis;
        // };

        auto key = makeConstraintsKey(constraints);

        auto& atomized = *node->atomized;
        auto& prelayout = *node->preLayout;

        SizeRequest sizeRequest = std::move(intrinsicSizeRequest).value_or(
            SizeRequest {
                .position = node->shared.position,
                .specified = {.width = node->shared.width, .height = node->shared.height},
                .override = constraints.parentOverride,
                .content = {.width = std::monostate{}, .height = std::monostate{}},
                .minimum = {.width = node->shared.minWidth, .height = node->shared.minHeight},
                .maximum = {
                    .width = node->shared.maxWidth ? SizeState{*node->shared.maxWidth} : SizeState{std::monostate{}},
                    .height = node->shared.maxHeight ? SizeState{*node->shared.maxHeight} : SizeState{std::monostate{}},
                },
                .available = {.width = constraints.availableWidth, .height = constraints.availableHeight},
                .top = node->shared.top,
                .right = node->shared.right,
                .bottom = node->shared.bottom,
                .left = node->shared.left,
                .paddingTop = node->shared.paddingTop.value_or(node->shared.padding),
                .paddingRight = node->shared.paddingRight.value_or(node->shared.padding),
                .paddingBottom = node->shared.paddingBottom.value_or(node->shared.padding),
                .paddingLeft = node->shared.paddingLeft.value_or(node->shared.padding),
                .borderWidth = node->shared.borderWidth,
                .margins = prelayout.resolvedMargins,
                .aspectRatio = node->shared.aspectRatio,
                .automaticWidth = constraints.shrinkWidthToFit ? AutomaticSizing::UseContent : AutomaticSizing::UseAvailable,
                .automaticHeight = AutomaticSizing::UseContent,
                .automaticMinimumWidth = AutomaticMinimum::Zero,
                .automaticMinimumHeight = AutomaticMinimum::Zero,
            }
        );

        auto sizeResult = evaluateSize(*this, node, frameInfo, constraints, measured, sizeRequest);

        const auto* resolvedWidth = std::get_if<float>(&sizeResult.size.width);
        if (resolvedWidth) {
            measured.explicitWidth = *resolvedWidth;
        }

        const auto* resolvedHeight = std::get_if<float>(&sizeResult.size.height);
        if (resolvedHeight) {
            measured.explicitHeight = *resolvedHeight;
        }

        constraints.resolvedMargins = prelayout.resolvedMargins;

        // what i should do now:
        // make this take in a size result instead of doing the computation separately
        auto layout = node->element->layout(constraints, node->shared, measured, atomized);

        auto childConstraints = layout.childConstraints;
        childConstraints.inheritedProperties = constraints.inheritedProperties;
        childConstraints.textOverflow = constraints.textOverflow;

        if (node->shared.overflow != Overflow::Visible) {
            childConstraints.textOverflow = node->shared.textOverflow;
        }

        if (node->getPosition() != Position::Static) {
            childConstraints.absoluteContainingBlock = {
                .origin = {0.0f, 0.0f},
                .width = layout.resolvedSize.width ? Size::px(layout.computedBox.width) : Size::autoSize(),
                .height = layout.resolvedSize.height ? Size::px(layout.computedBox.height) : Size::autoSize(),
            };
        } else {
            childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        }

        float minX = childConstraints.origin.x;
        float maxX = childConstraints.origin.x;
        float minY = childConstraints.origin.y;
        float maxY = childConstraints.origin.y;
        float minimumContent = 0.0f;
        float maximumContent = 0.0f;

        InlineSizingInput inlineSizing {
            .availableWidth = childConstraints.availableWidth,
            .widthRequest = std::nullopt,
            .trackIntrinsicWidth = sizeRequest.resolvingIntrinsicWidth,
        };
        auto inlineFormatting = buildInlineBoxes(node, inlineSizing);

        auto flexPass = [&]() {
            auto flexDirection = node->getFlexDirection();
            auto justifyContent = node->getJustifyContent();
            auto alignItems = node->getAlignItems();
            auto alignContentVal = node->getAlignContent();
            auto flexWrap = node->getFlexWrap();

            FlexLayout flexContext {flexDirection, justifyContent, alignItems, alignContentVal, flexWrap};
            flexContext.axis.applyDirection(constraints.inheritedProperties.direction);

            FlexResolver fr {
                *this, node, constraints, childConstraints, flexContext, frameInfo, measured,
                mutate, childConstraints.availableWidth, childConstraints.availableHeight,
                minX, minY, maxX, maxY
            };

            fr.phaseB();
            auto bounds = fr.phaseC();

            maxX = bounds.maxX;
            maxY = bounds.maxY;

            if (fr.intrinsicSizes) {
                minimumContent = std::max(minimumContent, fr.intrinsicSizes->minimum);
                maximumContent = std::max(maximumContent, fr.intrinsicSizes->maximum);
            }
        };


        auto gridPass = [&]() {
            GridResolver gr {
                *this, node, constraints, childConstraints, frameInfo, measured,
                mutate, childConstraints.availableWidth, childConstraints.availableHeight,
                minX, minY, maxX, maxY
            };

            gr.phaseB();

            auto bounds = gr.phaseC();

            maxX = bounds.maxX;
            maxY = bounds.maxY;
            if (gr.intrinsicSizes) {
                minimumContent = std::max(minimumContent, gr.intrinsicSizes->minimum);
                maximumContent = std::max(maximumContent, gr.intrinsicSizes->maximum);
            }
        };

        auto normalPass = [&]() {
            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto child = node->children[i].get();

                childConstraints.inlineFormatting = {
                    .context = inlineFormatting,
                    .fragments = inlineFormatting->childFragments[i],
                };

                auto childOutput = layoutRecursive(child, frameInfo, childConstraints, *child->measured, mutate);
                auto& childLayout = childOutput.layout;
                
                // track:
                // childOutput.intrinsicSizes-> min and max as min/max content
                // simultaneously, track the computed box content size
                // so we will get three things. not very hard.

                if (!childLayout.outOfFlow) {
                    if (childOutput.intrinsicSizes) {
                        minimumContent = std::max(minimumContent, childOutput.intrinsicSizes->minimum);
                        maximumContent = std::max(maximumContent, childOutput.intrinsicSizes->maximum);
                    }

                    childConstraints.cursor = childLayout.siblingCursor;
                    childConstraints.edgeIntent = childLayout.edgeIntent;
                    childConstraints.prevInlineHeight = childLayout.prevInlineHeight;

                    maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
                    maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
                }
            }
        };

        auto display = node->getDisplay();

        switch (display) {
            case style::Display::Flex: {
                flexPass();
                break;
            }
            case style::Display::Grid: {
                gridPass();
                break;
            }
            default: {
                normalPass();
                break;
            }
        }

        // correct intrinsic sizes if no children
        if (node->children.empty()) {
            float extent = 0.0;
            if (intrinsicSizeRequest->resolvingIntrinsicWidth || intrinsicSizeRequest->resolvingIntrinsicHeight) {
                extent = sizeRequest.resolvingIntrinsicWidth ? layout.computedBox.width : layout.computedBox.height;
            }
            minimumContent = extent;
            maximumContent = extent;
        }

        // create new req, with contentWidth/Height set
        float contentWidth = maxX - minX;
        float contentHeight = maxY - minY;

        SizeRequest resizeRequest = sizeRequest;
        resizeRequest.content = {
            .width = contentWidth,
            .height = contentHeight,
        };
        
        auto resizeResult = evaluateSize(*this, node, frameInfo, constraints, measured, resizeRequest);

        // check if size results MATCH with og one;
        const auto* initialWidth = std::get_if<float>(&sizeResult.size.width);
        const auto* initialHeight = std::get_if<float>(&sizeResult.size.height);
        const auto* resizedWidth = std::get_if<float>(&resizeResult.size.width);
        const auto* resizedHeight = std::get_if<float>(&resizeResult.size.height);

        bool widthChanged = resizedWidth && (!initialWidth || *resizedWidth != *initialWidth);
        bool heightChanged = resizedHeight && (!initialHeight || *resizedHeight != *initialHeight);

        // if there is deviation, rerun layout
        if (widthChanged || heightChanged) {
            if (widthChanged) {
                measured.explicitWidth = *resizedWidth;
            }
            if (heightChanged) {
                measured.explicitHeight = *resizedHeight;
            }

            layout = node->element->layout(constraints, node->shared, measured, atomized);

            childConstraints = layout.childConstraints;
            childConstraints.inheritedProperties = constraints.inheritedProperties;
            childConstraints.textOverflow = constraints.textOverflow;

            if (node->shared.overflow != Overflow::Visible) {
                childConstraints.textOverflow = node->shared.textOverflow;
            }

            if (node->getPosition() != Position::Static) {
                childConstraints.absoluteContainingBlock = {
                    .origin = {0.0f, 0.0f},
                    .width = layout.resolvedSize.width ? Size::px(layout.computedBox.width) : Size::autoSize(),
                    .height = layout.resolvedSize.height ? Size::px(layout.computedBox.height) : Size::autoSize(),
                };
            } else {
                childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
            }

            minX = childConstraints.origin.x;
            maxX = childConstraints.origin.x;
            minY = childConstraints.origin.y;
            maxY = childConstraints.origin.y;
            minimumContent = 0.0f;
            maximumContent = 0.0f;

            inlineSizing.availableWidth = childConstraints.availableWidth;
            inlineFormatting = buildInlineBoxes(node, inlineSizing);

            switch (display) {
                case style::Display::Flex: {
                    flexPass();
                    break;
                }
                case style::Display::Grid: {
                    gridPass();
                    break;
                }
                default: {
                    normalPass();
                    break;
                }
            }
        }

        layout.localComputedBox = layout.computedBox;
        layout.localAtomOffsets = layout.atomOffsets;

        LayoutOutput output {
            .layout = layout,
            .sizeResult = resizeResult,
            .intrinsicSizes = IntrinsicSizes {
                .minimum = minimumContent,
                .maximum = maximumContent
            }
        };


        if (mutate) {
            node->layout = output.layout;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }

        return output;
        // auto key = makeConstraintsKey(constraints);

        // if (mutate) {
        //     auto reason = recomputeReason(node, DirtyBits::Layout, key);
        //     if (reason == instrumentation::RecomputeReason::None) {
        //         reason = instrumentation::RecomputeReason::AncestorRecomputed;
        //     }
        //     instrumentation::recordRecompute(node->id, instrumentation::Phase::Layout, reason);
        // }

        // auto& atomized = *node->atomized;
        // auto& prelayout = *node->preLayout;

        // bool outOfFlow = node->shared.position == Position::Absolute || node->shared.position == Position::Fixed;
        // bool hadExplicitWidth = measured.explicitWidth.has_value() && (node->shared.width.unit == Unit::Px || node->shared.width.unit == Unit::Pt || (outOfFlow && node->shared.left.has_value() && node->shared.right.has_value()));
        // bool hadExplicitHeight = measured.explicitHeight.has_value() && (node->shared.height.unit == Unit::Px || node->shared.height.unit == Unit::Pt || (outOfFlow && node->shared.top.has_value() && node->shared.bottom.has_value()));
        // auto display = node->getDisplay();
        // bool isInline = node->element->isInline() && display == Display::Inline;
        // bool isNormalFlow = display != Display::Flex && display != Display::Grid;
        // SharedDescriptor computedShared = node->shared;
        // computedShared.display = display;

        // constraints.resolvedMargins = prelayout.resolvedMargins;
        // bool intrinsicPreferredWidth = !measured.explicitWidth && measured.explicitWidth.error() == SizeError::ContentDependent;
        // bool intrinsicPreferredHeight = !measured.explicitHeight && measured.explicitHeight.error() == SizeError::ContentDependent;

        // // I can replace axis res deferred with a parent override
        // // then if theres a parent override that goes into the req, we know to replace that
        // // this fucking guard is going to kill me (final)
        // // it literally makes this refactor damn enar fucking impossible
        
        // // // if we arent already resolving the width axis and we fit all the conditions, resolve width!
        // bool intrinsicHeightBounds = constraints.intrinsicSizesAxis != Axis::Height && (node->shared.minHeight.isContentDependent() || (node->shared.maxHeight.has_value() && node->shared.maxHeight->isContentDependent()));
        // std::optional<IntrinsicSizes> widthIntrinsicSizes;
        // std::optional<IntrinsicSizes> heightIntrinsicSizes;

        // SizeRequest sizeRequest {
        //     .specified = {.width = node->shared.width, .height = node->shared.height},
        //     .override = constraints.parentOverride,
        //     .content = {.width = std::monostate{}, .height = std::monostate{}},
        //     .minimum = {.width = node->shared.minWidth, .height = node->shared.minHeight},
        //     .maximum = {.width = node->shared.maxWidth.value_or(Size::autoSize()), .height = node->shared.maxHeight.value_or(Size::autoSize())},
        //     .available = {.width = constraints.availableWidth, .height = constraints.availableHeight},
        //     .margins = constraints.resolvedMargins,
        // };

        // auto sizeResult = evaluateSize(*this, node, frameInfo, constraints, measured, sizeRequest);

        // if (auto width = std::get_if<float>(&sizeResult.size.width))
        //     measured.explicitWidth = *width;
        // if (auto height = std::get_if<float>(&sizeResult.size.height))
        //     measured.explicitHeight = *height;

        // widthIntrinsicSizes = sizeResult.widthIntrinsicSizes;
        // heightIntrinsicSizes = sizeResult.heightIntrinsicSizes;

        // if (intrinsicPreferredWidth && widthIntrinsicSizes.has_value()) 
        //     measured.explicitWidth = layout::resolveIntrinsicSize(node->shared.width, *widthIntrinsicSizes, constraints.availableWidth);;

        // if (intrinsicPreferredHeight && heightIntrinsicSizes.has_value()) 
        //     measured.explicitHeight = layout::resolveIntrinsicSize(node->shared.height, *heightIntrinsicSizes, constraints.availableHeight);

        // // self-layout
        // auto layout = node->element->layout(constraints, computedShared, measured, atomized);

        // auto childConstraints = layout.childConstraints;
        // childConstraints.intrinsicSizesAxis = constraints.intrinsicSizesAxis;
        
        // // this is still doing... something? idk what tho
        // if (constraints.shrinkWidthToFit && node->shared.width.isAuto()) {
        //     childConstraints.shrinkWidthToFit = true;
        // }

        // if (constraints.shrinkHeightToFit && node->shared.height.isAuto()) {
        //     childConstraints.shrinkHeightToFit = true;
        // }

        // if (constraints.widthResolution == AxisResolution::MinContent ||
        //     constraints.widthResolution == AxisResolution::MaxContent) {
        //     childConstraints.widthResolution = constraints.widthResolution;
        // }

        // if (constraints.heightResolution == AxisResolution::MinContent ||
        //     constraints.heightResolution == AxisResolution::MaxContent) {
        //     childConstraints.heightResolution = constraints.heightResolution;
        // }
        
        // childConstraints.textOverflow = constraints.textOverflow;
        // if (node->shared.overflow != Overflow::Visible) {
        //     childConstraints.textOverflow = node->shared.textOverflow;
        // }
        // Size parentAvailableWidth = childConstraints.availableWidth;
        // Size parentAvailableHeight = childConstraints.availableHeight;
        // float originX = childConstraints.origin.x;
        // float originY = childConstraints.origin.y;

        // auto position = node->getPosition();
        // if (position != Position::Static) {
        //     childConstraints.absoluteContainingBlock = {
        //         .origin = {0.0f, 0.0f},
        //         .width = layout.resolvedSize.width ? Size::px(layout.computedBox.width) : Size::autoSize(),
        //         .height = layout.resolvedSize.height ? Size::px(layout.computedBox.height) : Size::autoSize()
        //     };
        // } else {
        //     childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        // }

        // float minY = originY;
        // float maxY = originY;
        // float minX = originX;
        // float maxX = originX;

        // auto inlineFormatting = buildInlineBoxes(node, childConstraints);
        // std::optional<IntrinsicSizes> intrinsicSizes;

        // auto normalPass = [&](){
        //     if (constraints.intrinsicSizesAxis == Axis::Width) {
        //         auto intrinsicContext = node->element->isInline() ? constraints.inlineFormatting.context : inlineFormatting;
        //         bool isolatedContext = intrinsicContext && constraints.inlineFormatting.fragments.start == 0 && constraints.inlineFormatting.fragments.count == intrinsicContext->fragments.size();
        //         intrinsicSizes = intrinsicContext && intrinsicContext->intrinsicSizes.has_value() && (!node->element->isInline() || isolatedContext) ? intrinsicContext->intrinsicSizes : std::nullopt;
        //         if (intrinsicSizes.has_value() && !isInline) {
        //             float horizontalPadding = layout.resolvedPadding.left + layout.resolvedPadding.right;
        //             intrinsicSizes->minContent = Size::px(intrinsicSizes->minContent.resolveOr(Size::autoSize()) + horizontalPadding);
        //             intrinsicSizes->maxContent = Size::px(intrinsicSizes->maxContent.resolveOr(Size::autoSize()) + horizontalPadding);
        //         }
        //     }

        //     for (uint64_t i = 0; i < node->children.size(); ++i) {
        //         auto& child = node->children[i];
        //         auto childAsPtr = child.get();

        //         childConstraints.inlineFormatting = {
        //             .context = inlineFormatting,
        //             .fragments = inlineFormatting->childFragments[i]
        //         };
        //         childConstraints.inheritedProperties = constraints.inheritedProperties;
        //         auto childOutput = layoutRecursive(
        //             childAsPtr,
        //             frameInfo,
        //             childConstraints,
        //             *childAsPtr->measured,
        //             mutate
        //         );
        //         auto& childLayout = childOutput.layout;
                
        //         if (!childLayout.outOfFlow) {
        //             if (constraints.intrinsicSizesAxis == Axis::Width && intrinsicSizes.has_value() && childAsPtr->getDisplay() != Display::Inline && childOutput.intrinsicSizes.has_value()) {
        //                 float leadingMargin = constraints.inheritedProperties.direction == layout::Direction::ltr ? childAsPtr->preLayout->resolvedMargins.left : childAsPtr->preLayout->resolvedMargins.right;
        //                 float childMinContent = childOutput.intrinsicSizes->minContent.resolveOr(Size::autoSize());
        //                 float childMaxContent = childOutput.intrinsicSizes->maxContent.resolveOr(Size::autoSize());
        //                 intrinsicSizes->minContent = Size::px(std::max(intrinsicSizes->minContent.resolveOr(Size::autoSize()), leadingMargin + childMinContent));
        //                 intrinsicSizes->maxContent = Size::px(std::max(intrinsicSizes->maxContent.resolveOr(Size::autoSize()), leadingMargin + childMaxContent));
        //             }


        //             childConstraints.cursor = childLayout.siblingCursor;
        //             childConstraints.edgeIntent = childLayout.edgeIntent;
        //             childConstraints.prevInlineHeight = childLayout.prevInlineHeight;
                
        //             maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
        //             maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        //         }
        //     }
        // };

        // auto flexPass = [&]() {
        //     auto flexDirection = node->getFlexDirection();
        //     auto justifyContent = node->getJustifyContent();
        //     auto alignItems = node->getAlignItems();
        //     auto alignContentVal = node->getAlignContent();
        //     auto flexWrap = node->getFlexWrap();

        //     FlexLayout flexContext {flexDirection, justifyContent, alignItems, alignContentVal, flexWrap};
        //     flexContext.axis.applyDirection(constraints.inheritedProperties.direction);

        //     FlexResolver fr {
        //         *this, node, constraints, childConstraints, flexContext, frameInfo, measured,
        //         mutate,
        //         parentAvailableWidth, parentAvailableHeight, minX, minY, maxX, maxY
        //     };

        //     fr.phaseB();
        //     auto bounds = fr.phaseC();

        //     maxX = bounds.maxX;
        //     maxY = bounds.maxY;
        //     intrinsicSizes = fr.intrinsicSizes;
        //     if (intrinsicSizes.has_value()) {
        //         float padding = constraints.intrinsicSizesAxis == Axis::Width ? layout.resolvedPadding.left + layout.resolvedPadding.right : layout.resolvedPadding.top + layout.resolvedPadding.bottom;
        //         intrinsicSizes->minContent = Size::px(intrinsicSizes->minContent.resolveOr(Size::autoSize()) + padding);
        //         intrinsicSizes->maxContent = Size::px(intrinsicSizes->maxContent.resolveOr(Size::autoSize()) + padding);
        //     }
        // };

        // auto gridPass = [&]() {
        //     GridResolver gr {
        //         *this, node, constraints, childConstraints, frameInfo, measured,
        //         mutate,
        //         parentAvailableWidth, parentAvailableHeight, minX, minY, maxX, maxY
        //     };

        //     gr.phaseB();

        //     auto bounds = gr.phaseC();


        //     maxX = bounds.maxX;
        //     maxY = bounds.maxY;
        //     intrinsicSizes = gr.intrinsicSizes;
        //     if (intrinsicSizes.has_value()) {
        //         float padding = constraints.intrinsicSizesAxis == Axis::Width ? layout.resolvedPadding.left + layout.resolvedPadding.right : layout.resolvedPadding.top + layout.resolvedPadding.bottom;
        //         intrinsicSizes->minContent = Size::px(intrinsicSizes->minContent.resolveOr(Size::autoSize()) + padding);
        //         intrinsicSizes->maxContent = Size::px(intrinsicSizes->maxContent.resolveOr(Size::autoSize()) + padding);
        //     }
        // };

        // if (display == Display::Flex) {
        //     flexPass();
        // } else if (display == Display::Grid) {
        //     gridPass();
        // } else if (isNormalFlow) {
        //     normalPass();
        // }


        // float contentWidth = maxX - minX + layout.resolvedPadding.left + layout.resolvedPadding.right;
        // float contentHeight = maxY - minY + layout.resolvedPadding.top + layout.resolvedPadding.bottom;

        // // overall what are we doing here?
        // // we are determining if the content width _changed_ the expected sizes
        // // if the size was specified
        // // then resolved size was already determiend and thus content width CANNOT have an effect
        

        // // i need to pass the content width
        // // i need to *think* about how to redo height intrinsic sizing...
        // // because here it relies on the used width while elsewhere it does not


        // SizeRequest contentSizeRequest {
        //     .specified = {.width = contentWidth, .height = contentHeight},
        //     .override = constraints.parentOverride,
        //     .content = {.width = std::monostate{}, .height = std::monostate{}},
        //     .minimum = {.width = node->shared.minWidth, .height = node->shared.minHeight},
        //     .maximum = {.width = node->shared.maxWidth.value_or(Size::autoSize()), .height = node->shared.maxHeight.value_or(Size::autoSize())},
        //     .available = {.width = constraints.availableWidth, .height = constraints.availableHeight},
        //     .margins = constraints.resolvedMargins,
        // };

        // auto contentSizeResult = evaluateSize(*this, node, frameInfo, constraints, measured, sizeRequest);



        // // set intrinsic height results
        // // first: setting intrinsic height to computed box (mainly for leaf nodes and propagates upwards)
        // // record these intrinsic sizes so parents can use them
        // if (constraints.intrinsicSizesAxis == Axis::Height && isNormalFlow) {
        //     float intrinsicHeight = layout.computedBox.height;
        //     intrinsicSizes = IntrinsicSizes{.minContent = Size::px(intrinsicHeight), .maxContent = Size::px(intrinsicHeight)};
        // }

        // // if our resolved sizes _dont exist_, we didnt resolve a size (auto?)
        // // so in that case, we use the computed box as scratch work and record the content values as sizes...
        // // not very clear to me but it wont get any clearer
        // if (!layout.resolvedSize.width) {
        //     layout.computedBox.width = contentWidth;
        // }
        // if (!layout.resolvedSize.height) {
        //     layout.computedBox.height = contentHeight;
        //     if (!layout.outOfFlow) {
        //         layout.consumedHeight = contentHeight;
        //     }
        // }

        // // start recording a "used" width, which starts as the computed box (we know it'll have been finalized)
        // // as either the computed box or content sizes
        // // then we slowly clamp it, transform it, etc...

        // // one idea: why not make a new sizing request with the content width/height
        // // but that isnt what usedWidth/height really are?
        // float usedWidth = std::visit(Overloaded{
        //     [](float width) { return width; },
        //     [&](const auto& other) { 
        //         std::println("other type: {}", typeid(other).name());
        //         return contentWidth; 
        //     }
        // }, sizeResult.size.width);

        // float usedHeight = std::visit(Overloaded{
        //     [](float height) { return height; },
        //     [&](const auto&) { return contentHeight; }
        // }, sizeResult.size.height);

        // bool retryWidth = std::holds_alternative<float>(sizeResult.size.width) && usedWidth != std::get<float>(sizeResult.size.width);
        // bool retryHeight = std::holds_alternative<float>(sizeResult.size.height) && usedHeight != std::get<float>(sizeResult.size.height);

        // // if (std::holds_alternative<std::monostate>(constraints.parentOverride.width)) {
        // //     if (node->shared.maxWidth.has_value()) {
        // //         const auto& requestedMaxWidth = *node->shared.maxWidth;
        // //         float maxWidth = requestedMaxWidth.isContentDependent() && widthIntrinsicSizes.has_value() ? layout::resolveIntrinsicSize(requestedMaxWidth, *widthIntrinsicSizes, constraints.availableWidth) : requestedMaxWidth.resolveOr(constraints.availableWidth, usedWidth);
        // //         usedWidth = std::min(usedWidth, maxWidth);
        // //     }

        // //     float minWidth = node->shared.minWidth.isContentDependent() && widthIntrinsicSizes.has_value() ? layout::resolveIntrinsicSize(node->shared.minWidth, *widthIntrinsicSizes, constraints.availableWidth) : node->shared.minWidth.resolveOr(constraints.availableWidth, usedWidth);
        // //     usedWidth = std::max(usedWidth, minWidth);
        // // }

        // // if (std::holds_alternative<std::monostate>(constraints.parentOverride.height)) {
        // //     // if our min/max height are intrinsically defined
        // //     // then at this point we redo our intrinisc heihgt measurement
        // //     // based on our current used width
        // //     if (intrinsicHeightBounds) {
        // //         Measured intrinsicMeasured = measured;
        // //         intrinsicMeasured.explicitWidth = usedWidth;
        // //         heightIntrinsicSizes = measureIntrinsicSizes(node, frameInfo, constraints, intrinsicMeasured, Axis::Height);
        // //     }

        // //     if (node->shared.maxHeight.has_value()) {
        // //         const auto& requestedMaxHeight = *node->shared.maxHeight;
        // //         float maxHeight = requestedMaxHeight.isContentDependent() && heightIntrinsicSizes.has_value() ? layout::resolveIntrinsicSize(requestedMaxHeight, *heightIntrinsicSizes, constraints.availableHeight) : requestedMaxHeight.resolveOr(constraints.availableHeight, usedHeight);
        // //         usedHeight = std::min(usedHeight, maxHeight);
        // //     }

        // //     float minHeight = node->shared.minHeight.isContentDependent() && heightIntrinsicSizes.has_value() ? layout::resolveIntrinsicSize(node->shared.minHeight, *heightIntrinsicSizes, constraints.availableHeight) : node->shared.minHeight.resolveOr(constraints.availableHeight, usedHeight);
        // //     usedHeight = std::max(usedHeight, minHeight);
        // // }

        // // if (layout.resolvedSize.width)
        // //     layout.resolvedSize.width = usedWidth;

        // // if (layout.resolvedSize.height)
        // //     layout.resolvedSize.height = usedHeight;

        // // // ignoring aspect ratio for now;
        // // // if (node->shared.aspectRatio)
        // // //     layout::transferAspectRatio(
        // // //         layout.resolvedSize.width,
        // // //         layout.resolvedSize.height,
        // // //         *node->shared.aspectRatio
        // // //     );

        // // usedWidth = layout.resolvedSize.width.value_or(usedWidth);
        // // usedHeight = layout.resolvedSize.height.value_or(usedHeight);

        // if (retryWidth || retryHeight) {
        //     Measured retryMeasured = measured;
        //     if (retryWidth) retryMeasured.explicitWidth = usedWidth;
        //     if (retryHeight) retryMeasured.explicitHeight = usedHeight;
            
        //     auto retryInput = layout::toLayoutInput(computedShared, retryMeasured);
        //     constraints.resolvedMargins = layout::LayoutEngine::resolveAutoMargins(
        //         retryInput,
        //         constraints.replacedAttributes,
        //         constraints.availableWidth,
        //         usedWidth
        //     );

        //     layout = node->element->layout(constraints, computedShared, retryMeasured, atomized);
            
        //     childConstraints = layout.childConstraints;
        //     childConstraints.intrinsicSizesAxis = constraints.intrinsicSizesAxis;

        //     if (constraints.shrinkWidthToFit && node->shared.width.isAuto()) {
        //         childConstraints.shrinkWidthToFit = true;
        //     }
        //     if (constraints.shrinkHeightToFit && node->shared.height.isAuto()) {
        //         childConstraints.shrinkHeightToFit = true;
        //     }
        //     if (constraints.widthResolution == AxisResolution::MinContent ||
        //         constraints.widthResolution == AxisResolution::MaxContent) {
        //         childConstraints.widthResolution = constraints.widthResolution;
        //     }
        //     if (constraints.heightResolution == AxisResolution::MinContent ||
        //         constraints.heightResolution == AxisResolution::MaxContent) {
        //         childConstraints.heightResolution = constraints.heightResolution;
        //     }
        //     childConstraints.textOverflow = constraints.textOverflow;
        //     if (node->shared.overflow != Overflow::Visible) {
        //         childConstraints.textOverflow = node->shared.textOverflow;
        //     }
        //     parentAvailableWidth  = childConstraints.availableWidth;
        //     parentAvailableHeight = childConstraints.availableHeight;

        //     if (position != Position::Static) {
        //         childConstraints.absoluteContainingBlock = {
        //             .origin = {0.0f, 0.0f},
        //             .width = layout.resolvedSize.width ? Size::px(layout.computedBox.width) : Size::autoSize(),
        //             .height = layout.resolvedSize.height ? Size::px(layout.computedBox.height) : Size::autoSize()
        //         };
        //     } else {
        //         childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        //     }

        //     originX = childConstraints.origin.x;
        //     originY = childConstraints.origin.y;
        //     minY = maxY = originY;
        //     minX = maxX = originX;

        //     inlineFormatting = buildInlineBoxes(node, childConstraints);

        //     if (display == Display::Flex) {
        //         flexPass();
        //     } else if (display == Display::Grid) {
        //         gridPass();
        //     } else if (isNormalFlow) {
        //         normalPass();
        //     }

        //     float retryContentHeight = maxY - minY + layout.resolvedPadding.top + layout.resolvedPadding.bottom;
        //     if (constraints.intrinsicSizesAxis == Axis::Height && isNormalFlow) {
        //         float intrinsicHeight = isInline || node->children.empty() ? layout.computedBox.height : retryContentHeight;
        //         intrinsicSizes = IntrinsicSizes{.minContent = Size::px(intrinsicHeight), .maxContent = Size::px(intrinsicHeight)};
        //     }

        //     if (retryWidth) {
        //         layout.computedBox.width = usedWidth;
        //     } else if (!layout.resolvedSize.width) {
        //         layout.computedBox.width = maxX - minX + layout.resolvedPadding.left + layout.resolvedPadding.right;
        //     }

        //     if (retryHeight) {
        //         layout.computedBox.height = usedHeight;
        //     } else if (!layout.resolvedSize.height) {
        //         layout.computedBox.height = retryContentHeight;

        //         if (std::holds_alternative<std::monostate>(constraints.parentOverride.height)) {
        //             if (node->shared.maxHeight.has_value()) {
        //                 const auto& requestedMaxHeight = *node->shared.maxHeight;
        //                 float maxHeight = requestedMaxHeight.isContentDependent() && heightIntrinsicSizes.has_value() ? layout::resolveIntrinsicSize(requestedMaxHeight, *heightIntrinsicSizes, constraints.availableHeight) : requestedMaxHeight.resolveOr(constraints.availableHeight, layout.computedBox.height);
        //                 layout.computedBox.height = std::min(layout.computedBox.height, maxHeight);
        //             }

        //             float minHeight = node->shared.minHeight.isContentDependent() && heightIntrinsicSizes.has_value() ? layout::resolveIntrinsicSize(node->shared.minHeight, *heightIntrinsicSizes, constraints.availableHeight) : node->shared.minHeight.resolveOr(constraints.availableHeight, layout.computedBox.height);
        //             layout.computedBox.height = std::max(layout.computedBox.height, minHeight);
        //         }
        //     }

        //     if (!layout.outOfFlow) {
        //         layout.consumedHeight = layout.computedBox.height;
        //     }
        // }

        // if (!layout.outOfFlow && display == Display::Block) {
        //     layout.siblingCursor.y = layout.computedBox.y + layout.consumedHeight;
        // }

        // // finalize layout of node
        // layout.localComputedBox = layout.computedBox;
        // layout.localAtomOffsets = layout.atomOffsets;
        // LayoutOutput output {
        //     .layout = std::move(layout),
        //     .intrinsicSizes = std::nullopt
        // };

        // if (constraints.intrinsicSizesAxis.has_value()) {
        //     Axis axis = *constraints.intrinsicSizesAxis;
        //     float maxContent = axis == Axis::Width ? output.layout.computedBox.width : output.layout.computedBox.height;
        //     bool hadExplicitSize = axis == Axis::Width ? hadExplicitWidth : hadExplicitHeight;
        //     if (hadExplicitSize || !intrinsicSizes.has_value()) intrinsicSizes = IntrinsicSizes{.minContent = Size::px(maxContent), .maxContent = Size::px(maxContent)};
        //     output.intrinsicSizes = intrinsicSizes;
        // }

        // if (mutate) {
        //     node->layout = output.layout;
        //     node->constraintsKey = key;
        //     node->dirtySelf |= DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        // }

        // return output;
    }

    void RenderTree::postLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints,
                                      simd_float2 parentGlobalOrigin, simd_float2 absBlockGlobalOrigin) {
        auto key = makeConstraintsKey(constraints, parentGlobalOrigin, absBlockGlobalOrigin);
        auto reason = recomputeReason(node, DirtyBits::PostLayout, key);
        if (reason == instrumentation::RecomputeReason::None) {
            return;
        }
        instrumentation::recordRecompute(node->id, instrumentation::Phase::PostLayout, reason);

        auto& layout = *node->layout;
        layout.computedBox = layout.localComputedBox;
        layout.atomOffsets = layout.localAtomOffsets;

        auto position = node->getPosition();

        auto& dp = layout.deferredPosition;
        if (dp.right) {
            auto containingBlockWidth = dp.containingBlockWidth.resolve(Size::autoSize());
            auto right = dp.right->resolve(dp.containingBlockWidth);
            if (containingBlockWidth && right) {
                float newX = *containingBlockWidth - dp.marginRight - layout.computedBox.width - *right;
                float deltaX = newX - layout.computedBox.x;
                layout.computedBox.x = newX;
                for (auto& offset : layout.atomOffsets) offset.x += deltaX;
            }
        }

        if (dp.bottom) {
            auto containingBlockHeight = dp.containingBlockHeight.resolve(Size::autoSize());
            auto bottom = dp.bottom->resolve(dp.containingBlockHeight);
            if (containingBlockHeight && bottom) {
                float newY = *containingBlockHeight - dp.marginBottom - layout.computedBox.height - *bottom;
                float deltaY = newY - layout.computedBox.y;
                layout.computedBox.y = newY;
                for (auto& offset : layout.atomOffsets) offset.y += deltaY;
            }
        }

        simd_float2 baseOrigin;
        if (position == Position::Fixed) {
            baseOrigin = {0.0f, 0.0f};
        } else if (position == Position::Absolute) {
            baseOrigin = absBlockGlobalOrigin;
        } else {
            baseOrigin = parentGlobalOrigin;
        }

        layout.computedBox.x += baseOrigin.x;
        layout.computedBox.y += baseOrigin.y;
        for (auto& offset : layout.atomOffsets) {
            offset.x += baseOrigin.x;
            offset.y += baseOrigin.y;
        }
        node->globalOffset = baseOrigin;
        layout.clipUniforms = constraints.clipUniforms;

        if (node->shared.overflow == Overflow::Scroll) {
            float viewportLeft = layout.computedBox.x;
            float viewportRight = layout.computedBox.x + layout.computedBox.width;
            float viewportTop = layout.computedBox.y;
            float viewportBottom = layout.computedBox.y + layout.computedBox.height;

            for (auto& clip : constraints.clipUniforms) {
                viewportLeft = std::max(viewportLeft, clip.rectCenter.x - clip.halfExtent.x);
                viewportRight = std::min(viewportRight, clip.rectCenter.x + clip.halfExtent.x);
                viewportTop = std::max(viewportTop, clip.rectCenter.y - clip.halfExtent.y);
                viewportBottom = std::min(viewportBottom, clip.rectCenter.y + clip.halfExtent.y);
            }

            viewportLeft += layout.resolvedPadding.left;
            viewportRight -= layout.resolvedPadding.right;
            viewportTop += layout.resolvedPadding.top;
            viewportBottom -= layout.resolvedPadding.bottom;

            node->scrollViewportSize = {
                std::max(0.0f, viewportRight - viewportLeft),
                std::max(0.0f, viewportBottom - viewportTop)
            };
        }

        node->atomized = node->element->postLayout(constraints, node->shared, *node->measured,
                                                    *node->atomized, layout);

        simd_float2 currContentOrigin = {
            layout.computedBox.x + layout.resolvedPadding.left,
            layout.computedBox.y + layout.resolvedPadding.top
        };

        if (node->shared.overflow == Overflow::Scroll) {
            currContentOrigin.x += constraints.inheritedProperties.direction == layout::Direction::rtl
                ? node->scrollOffset.x
                : -node->scrollOffset.x;
            currContentOrigin.y -= node->scrollOffset.y;
        }

        simd_float2 childAbsBlockOrigin = absBlockGlobalOrigin;
        if (position != Position::Static) {
            childAbsBlockOrigin = currContentOrigin;
        }

        auto childConstraints = constraints;
        childConstraints.availableWidth = layout.childConstraints.availableWidth;
        if (node->shared.overflow != Overflow::Visible) {
            childConstraints.textOverflow = node->shared.textOverflow;
        }
        if (node->shared.overflow != Overflow::Visible) {
            float cornerRadius = node->shared.cornerRadius.resolveOr(
                Size::px(std::min(layout.computedBox.width, layout.computedBox.height))
            );

            simd_float2 halfExtent {
                layout.computedBox.width * 0.5f,
                layout.computedBox.height * 0.5f
            };

            childConstraints.clipUniforms.push_back({
                .rectCenter = {
                    layout.computedBox.x + halfExtent.x,
                    layout.computedBox.y + halfExtent.y
                },
                .halfExtent = halfExtent,
                .cornerRadius = {cornerRadius, cornerRadius}
            });

        }

        for (auto& child : node->children) {
            postLayoutPhase(child.get(), frameInfo, childConstraints,
                           currContentOrigin, childAbsBlockOrigin);
        }

        if (node->shared.overflow == Overflow::Scroll) {
            simd_float2 contentSize {0.0f, 0.0f};
            std::function<void(TreeNode*)> includeChildOverflow;
            includeChildOverflow = [&](TreeNode* child) {
                if (!child->layout.has_value() || child->layout->outOfFlow) return;
                auto& childBox = child->layout->computedBox;
                if (constraints.inheritedProperties.direction == layout::Direction::rtl) {
                    contentSize.x = std::max(
                        contentSize.x,
                        currContentOrigin.x + node->scrollViewportSize.x - childBox.x
                    );
                } else {
                    contentSize.x = std::max(contentSize.x, childBox.x + childBox.width - currContentOrigin.x);
                }
                contentSize.y = std::max(contentSize.y, childBox.y + childBox.height - currContentOrigin.y);

                if (child->shared.overflow != Overflow::Visible) return;
                for (auto& grandchild : child->children) {
                    includeChildOverflow(grandchild.get());
                }
            };

            for (auto& child : node->children) {
                includeChildOverflow(child.get());
            }
            node->scrollContentSize = contentSize;
        }

        node->constraintsKey = key;
        node->dirtySelf |= DirtyBits::Place | DirtyBits::Finalize;
    }

    void RenderTree::placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        auto key = makeConstraintsKey(constraints);
        auto reason = recomputeReason(node, DirtyBits::Place, key);
        if (reason != instrumentation::RecomputeReason::None) {
            instrumentation::recordRecompute(node->id, instrumentation::Phase::Place, reason);
            auto& measured = *node->measured;
            auto& atomized = *node->atomized;
            auto& layout = *node->layout;

            auto placed = node->element->place(constraints, node->shared, measured, atomized, layout);
            node->placed = placed;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::Finalize;
        }

        for (auto& child : node->children) {
            placePhase(child.get(), frameInfo, constraints);
        }
    }

    void RenderTree::finalizePhase(TreeNode* node, Constraints& constraints) {
        auto key = makeConstraintsKey(constraints);
        auto reason = recomputeReason(node, DirtyBits::Finalize, key);
        if (reason != instrumentation::RecomputeReason::None) {
            instrumentation::recordRecompute(node->id, instrumentation::Phase::Finalize, reason);
            auto& measured =  *node->measured;
            auto& atomized = *node->atomized;
            auto& layout = *node->layout;
            auto& placed = *node->placed;
            auto finalized = node->element->finalize(constraints, node->shared, measured, atomized, layout, placed);
            node->finalized = finalized;
            node->constraintsKey = key;
        }

        for (auto& child : node->children) {
            finalizePhase(child.get(), constraints);
        }
    }

    TreeNode* RenderTree::hitTestRecursive(TreeNode* node, simd_float2 point) {
        auto startedAt = std::chrono::steady_clock::time_point{};
        if constexpr (instrumentation::enabled) {
            startedAt = std::chrono::steady_clock::now();
        }
        uint64_t nodesExamined = 0;
        TreeNode* hit = nullptr;

        if (node) {
            auto& renderOrder = sortedRenderOrder();
            for (auto it = renderOrder.rbegin(); it != renderOrder.rend(); ++it) {
                auto* candidate = *it;
                auto isInSubtree = node->paintPreorderIndex <= candidate->paintPreorderIndex
                    && candidate->paintPostorderIndex <= node->paintPostorderIndex;
                if (!isInSubtree) {
                    continue;
                }

                nodesExamined++;
                if (candidate->contains(point)) {
                    hit = candidate;
                    break;
                }
            }
        }

        if constexpr (instrumentation::enabled) {
            instrumentation::recordHitTest(
                nodesExamined,
                hit ? 1 : 0,
                std::chrono::steady_clock::now() - startedAt
            );
        }
        return hit;
    }

    std::vector<TreeNode*> RenderTree::hitTestAll(simd_float2 point) {
        auto startedAt = std::chrono::steady_clock::time_point{};
        if constexpr (instrumentation::enabled) {
            startedAt = std::chrono::steady_clock::now();
        }
        std::vector<TreeNode*> hits;
        auto& renderOrder = sortedRenderOrder();

        for (auto it = renderOrder.rbegin(); it != renderOrder.rend(); ++it) {
            if ((*it)->contains(point)) {
                hits.push_back(*it);
            }
        }

        if constexpr (instrumentation::enabled) {
            instrumentation::recordHitTest(
                renderOrder.size(),
                hits.size(),
                std::chrono::steady_clock::now() - startedAt
            );
        }

        return hits;
    }
}
