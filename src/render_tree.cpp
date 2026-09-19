#include "render_tree.hpp"
#include "hash_combine.hpp"
#include "layout/layout.hpp"
#include "layout/sizing.hpp"
#include "layout/style.hpp"
#include "overloaded.hpp"
#include "layout/sizing.hpp"
#include "tree_node.hpp"
#include <algorithm>
#include <chrono>
#include <optional>
#include <print>
#include <utility>
#include <variant>

namespace tree {
    using layout::ContainingBlock;
    using layout::FlexLayout;
    using layout::FlexResolver;
    using layout::GridResolver;
    using layout::LayoutResult;
    using layout::MarginMetadata;
    using layout::IntrinsicSizes;
    using style::ClipUniform;

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

    ConstraintsKey RenderTree::makeConstraintsKey(const Constraints& constraints) const {
        std::size_t hash = 0;
        hash_combine(hash, constraints.origin.x);
        hash_combine(hash, constraints.origin.y);
        hash_combine(hash, constraints.cursor.x);
        hash_combine(hash, constraints.cursor.y);
        hashSize(constraints.availableWidth, hash);
        hashSize(constraints.availableHeight, hash);
        hash_combine(hash, static_cast<int>(constraints.inheritedProperties.direction));
        hash_combine(hash, static_cast<int>(constraints.inheritedProperties.textAlign));
        hash_combine(hash, constraints.frameInfo.width);
        hash_combine(hash, constraints.frameInfo.height);
        hash_combine(hash, constraints.frameInfo.scale);
        hash_combine(hash, constraints.containingBlock.origin.x);
        hash_combine(hash, constraints.containingBlock.origin.y);
        hashSize(constraints.containingBlock.width, hash);
        hashSize(constraints.containingBlock.height, hash);
        hash_combine(hash, constraints.absoluteContainingBlock.origin.x);
        hash_combine(hash, constraints.absoluteContainingBlock.origin.y);
        hashSize(constraints.absoluteContainingBlock.width, hash);
        hashSize(constraints.absoluteContainingBlock.height, hash);
        hash_combine(hash, constraints.fixedContainingBlock.origin.x);
        hash_combine(hash, constraints.fixedContainingBlock.origin.y);
        hashSize(constraints.fixedContainingBlock.width, hash);
        hashSize(constraints.fixedContainingBlock.height, hash);
        hash_combine(hash, constraints.scrollport.origin.x);
        hash_combine(hash, constraints.scrollport.origin.y);
        hashSize(constraints.scrollport.width, hash);
        hashSize(constraints.scrollport.height, hash);
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
            hash_combine(hash, clip.cornerRadius.topLeft.x);
            hash_combine(hash, clip.cornerRadius.topLeft.y);
            hash_combine(hash, clip.cornerRadius.topRight.x);
            hash_combine(hash, clip.cornerRadius.topRight.y);
            hash_combine(hash, clip.cornerRadius.bottomRight.x);
            hash_combine(hash, clip.cornerRadius.bottomRight.y);
            hash_combine(hash, clip.cornerRadius.bottomLeft.x);
            hash_combine(hash, clip.cornerRadius.bottomLeft.y);
        }

        return ConstraintsKey{.value = hash};
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

        uint64_t paintOrderIndex = 0;
        assignPaintOrderIndices(getRoot(), paintOrderIndex);

        renderOrderCache.clear();
        createStackingContexts(getRoot(), renderOrderCache);

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
        rootConstraints = Constraints {
            .origin = simd_float2{0,0},
            .cursor = rootCursor,
            .availableWidth = frameInfo.width,
            .availableHeight = frameInfo.height,
            .frameInfo = frameInfo,
            .containingBlock = {
                .origin = {0, 0},
                .width = frameInfo.width,
                .height = frameInfo.height
            },
            .absoluteContainingBlock = {
                .origin = {0, 0},
                .width = frameInfo.width,
                .height = frameInfo.height,
                .clipCount = 1
            },
            .fixedContainingBlock = {
                .origin = {0, 0},
                .width = frameInfo.width,
                .height = frameInfo.height,
                .clipCount = 1
            },
            .scrollport = {
                .origin = {0, 0},
                .width = frameInfo.width,
                .height = frameInfo.height
            },
            .clipUniforms = {
                ClipUniform {
                    .rectCenter = {frameInfo.width * 0.5f, frameInfo.height * 0.5f},
                    .halfExtent = {frameInfo.width * 0.5f, frameInfo.height * 0.5f},
                    .cornerRadius = {}
                }
            },
        };

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
            layoutCache.clear();
            sizeCache.clear();
            instrumentation::PhaseTimer timer{instrumentation::Phase::Layout};
            layoutPhase(root, frameInfo, rootConstraints);
        }

        sortedRenderOrder();
        // postLayout: resolve global positions (serial, top-down) + reconcile atoms
        if (subtreeHasDirty(root, DirtyBits::PostLayout) || !root->layout.has_value()) {
            instrumentation::PhaseTimer timer{instrumentation::Phase::PostLayout};
            postLayoutPhase(root, frameInfo, rootConstraints);
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
                atomCount += atomized.usesDrawableAtoms ? atomized.drawableAtoms.size() : atomized.atoms.size();
            }
            auto& finalized = node->finalized;
            node->element->encode(encoder, finalized);
        }
        instrumentation::recordRenderWork(allNodes.size(), allNodes.size(), atomCount);
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
            auto& shared = node->shared;
            auto atomized = node->element->atomize(constraints, shared);
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
            childConstraints.textBidiInput = (*childBidiInputs)[i];
            auto result = atomizePhase(node->children[i].get(), childConstraints);
            if (!result) return result;
        }
        return {};
    }

    void resolveComputedDisplays(TreeNode* node)
    {
        node->computedDisplay = node->shared.display;

        bool blockifyingParent = node->parent
            && (node->parent->getDisplay() == Display::Flex || node->parent->getDisplay() == Display::Grid);

        if (blockifyingParent && node->element->isReplaced() && node->shared.display == Display::Inline) {
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

        precomputeMargins(*this, node, constraints, collapsedChainMap);
    }

    // this should exist for entry pt reasons; makes sense
    void RenderTree::layoutPhase(
        TreeNode* node,
        const FrameInfo& frameInfo,
        Constraints constraints
    ) {
        layoutRecursive(node, frameInfo, constraints, true);
    }

    LayoutResult RenderTree::layoutRecursive(
        TreeNode* node,
        const FrameInfo& frameInfo,
        Constraints constraints,
        bool mutate,
        std::optional<SizeRequest> sizeRequestOverride, // not a fan of these two sources of truth existing
        std::optional<IntrinsicRequest> intrinsicWidthRequestOverride,
        std::optional<IntrinsicRequest> intrinsicHeightRequestOverride
    ) {
        auto key = makeConstraintsKey(constraints);

        auto& atomized = *node->atomized;
        auto& prelayout = *node->preLayout;

        SizeRequest sizeRequest = sizeRequestOverride
            ? std::move(*sizeRequestOverride)
            : SizeRequest {
                .position = node->shared.position,
                .specified = {.width = node->shared.width, .height = node->shared.height},
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
                .borderWidth = node->shared.border.width,
                .margins = prelayout.resolvedMargins,
                .aspectRatio = node->shared.aspectRatio,
                .automaticWidth = (node->getPosition() == Position::Absolute || node->getPosition() == Position::Fixed) ? AutomaticSizing::UseContent : AutomaticSizing::UseAvailable,
                .automaticHeight = AutomaticSizing::UseContent,
                .automaticMinimumWidth = AutomaticMinimum::Zero,
                .automaticMinimumHeight = AutomaticMinimum::Zero,
            };

        if (intrinsicWidthRequestOverride) {
            sizeRequest.intrinsicWidthRequest = intrinsicWidthRequestOverride;
            sizeRequest.resolvingIntrinsicWidth = true;
        }

        if (intrinsicHeightRequestOverride) {
            sizeRequest.intrinsicHeightRequest = intrinsicHeightRequestOverride;
            sizeRequest.resolvingIntrinsicHeight = true;
        }

        ConstraintsKey layoutKey = key;
        hash_combine(layoutKey.value, node->id);
        hashSizeRequest(sizeRequest, layoutKey.value);

        auto cachedLayout = layoutCache.find(layoutKey);
        // we dont need a mutation; just fetch the cached result
        // that being said idk if I care about this cache existing
        if (!mutate) {
            instrumentation::recordSpeculativeLayoutCache(cachedLayout != layoutCache.end());
            if (cachedLayout != layoutCache.end()) {
                return cachedLayout->second;
            }
        }

        auto sizeResult = evaluateSize(*this, node, frameInfo, constraints, sizeRequest, sizeCache);


        constraints.resolvedMargins = prelayout.resolvedMargins;
        constraints.computedDisplay = node->getDisplay();

        // what i should do now:
        // make this take in a size result instead of doing the computation separately
        auto layout = node->element->layout(constraints, node->shared, atomized, sizeResult);

        auto childConstraints = std::visit([](const auto& state) { return state.childConstraints; }, layout);
        childConstraints.inheritedProperties = constraints.inheritedProperties;
        childConstraints.textOverflow = constraints.textOverflow;

        if (node->shared.overflow != Overflow::Visible) {
            childConstraints.textOverflow = node->shared.textOverflow;
        }

        // local during layout (so origin always 0.0; becomes non-local in post-layout)
        childConstraints.containingBlock = {
            .origin = {0.0f, 0.0f},
            .width = sizeResult.innerSize.width,
            .height = sizeResult.innerSize.height,
        };

        // here: absolute replaces its containing block if
        // not static or is transformed
        if (node->getPosition() != Position::Static || node->shared.transform) {
            childConstraints.absoluteContainingBlock = {
                .origin = {0.0f, 0.0f},
                .width = sizeResult.paddingBoxSize.width,
                .height = sizeResult.paddingBoxSize.height,
            };
        } else {
            childConstraints.absoluteContainingBlock = constraints.absoluteContainingBlock;
        }

        // here; fixed replaces its containing block if transformed
        if (node->shared.transform) {
            childConstraints.fixedContainingBlock = childConstraints.absoluteContainingBlock;
        } else {
            childConstraints.fixedContainingBlock = constraints.fixedContainingBlock;
        }

        if (node->shared.overflow == Overflow::Scroll) {
            childConstraints.scrollport = {
                .origin = {0.0f, 0.0f},
                .width = sizeResult.paddingBoxSize.width,
                .height = sizeResult.paddingBoxSize.height,
            };
        } else {
            childConstraints.scrollport = constraints.scrollport;
        }

        std::optional<IntrinsicSizes> intrinsicResult;

        if (sizeRequest.resolvingIntrinsicWidth || sizeRequest.resolvingIntrinsicHeight) {
            intrinsicResult = IntrinsicSizes{};
        }

        if (std::holds_alternative<layout::BlockState>(layout)) {
            if (std::holds_alternative<float>(sizeResult.outerSize.width)) {
                float outerWidth = std::get<float>(sizeResult.outerSize.width);

                if (sizeRequest.resolvingIntrinsicWidth) {
                    intrinsicResult = IntrinsicSizes {.minimum = outerWidth, .maximum = outerWidth};
                }
            }

            if (std::holds_alternative<float>(sizeResult.outerSize.height)) {
                float outerHeight = std::get<float>(sizeResult.outerSize.height);

                if (sizeRequest.resolvingIntrinsicHeight) {
                    intrinsicResult = IntrinsicSizes {.minimum = outerHeight, .maximum = outerHeight};
                }
            }
        }

        if (std::holds_alternative<layout::InlineState>(layout)) {
            const auto& inlineState = std::get<layout::InlineState>(layout);

            if (inlineState.widthIntrinsicSizes) {
                if (sizeRequest.resolvingIntrinsicWidth) {
                    intrinsicResult = *inlineState.widthIntrinsicSizes;
                }
            }

            if (inlineState.heightIntrinsicSizes) {
                if (sizeRequest.resolvingIntrinsicHeight) {
                    intrinsicResult = *inlineState.heightIntrinsicSizes;
                }
            }
        }


        /*
            the arch problem
            producers:
                - computedBox (text)
                - outerSize (divs)
                - child contributions
            when to collect:
                - always?
                - when requested

            when to collect:
                - I think the target is clear: when requested

            what to collection:
                I am fine with outer size and child contributions
                I am NOT fine with the computed box

                my plan to unify these would probably be:
                    initialization:
                    if outersize:
                        use outersize
                    else:
                        use whatever the layout pass derived
                    
                    then during normal pass:
                        collect child intrinsic contributions

                    make intrinsic sizes truly optional: because they get initialized with 0.0f along both axes:
                        minimumContent = 0.0f;
                        maximumContent = 0.0f;

                        these always resolve to a value; that is a *problem*

                        gate initialization between a size request check

                        by doing this; we can also get rid of the correction based on whether or not a node *has children*
                    
                problem: how does text communicate its intrinsic sizes?
                    - evaluate size call: 
                        - literally not possible; evaluate size has a different job
                    - layout = node->element->layout(constraints, node->shared, measured, atomized, resizeResult);
                        - this would only get *conditionally filled*
                        - block has no meaningful value to populate here.

                further design distnctions:
                    I want to describe layout modes as first class vs second class

                    first class: block and inline
                        every node gets laid out by itself as block or inline
                        these modes size their container AND define how to place children

                    second class: so far, flex and grid
                        these layout mdoes do not define how the container is sized; they define how the container places its children
                        these are subsets of first class sizing modes

            because of the new *centralized* sizing resolver
            we dont need to actually run resize; it will run resize itself when it needs the values
        */

        auto flexPass = [&](const SizeResult& sr) {
            auto flexDirection = node->getFlexDirection();
            auto justifyContent = node->getJustifyContent();
            auto alignItems = node->getAlignItems();
            auto alignContentVal = node->getAlignContent();
            auto flexWrap = node->getFlexWrap();

            FlexLayout flexContext {flexDirection, justifyContent, alignItems, alignContentVal, flexWrap};
            flexContext.axis.applyDirection(constraints.inheritedProperties.direction);

            // temp variable for padding included available size

            FlexResolver fr {
                *this, node, constraints, childConstraints, flexContext, frameInfo, sr.innerSize, 
                mutate, sizeCache
            };

            fr.phaseB();
            auto result = fr.phaseC();

            if (sizeRequest.resolvingIntrinsicWidth || sizeRequest.resolvingIntrinsicHeight) {
                const IntrinsicResult& intrinsicSizes = sizeRequest.resolvingIntrinsicWidth
                    ? (flexContext.axis.isRow ? result.mainIntrinsicSizes : result.crossIntrinsicSizes)
                    : (flexContext.axis.isRow ? result.crossIntrinsicSizes : result.mainIntrinsicSizes);

                intrinsicResult = IntrinsicSizes {
                    .minimum = std::get<float>(intrinsicSizes.minimum),
                    .maximum = std::get<float>(intrinsicSizes.maximum)
                };
            }
        };


        auto gridPass = [&](const SizeResult& sr) {
            GridResolver gr {
                *this, node, constraints, childConstraints, frameInfo, sr,
                mutate, sizeCache
            };

            gr.phaseB();

            gr.phaseC();
            if (sizeRequest.resolvingIntrinsicWidth || sizeRequest.resolvingIntrinsicHeight) {
                const IntrinsicSizes& intrinsicSizes = sizeRequest.resolvingIntrinsicWidth
                    ? gr.gridLayout.columnIntrinsicSizes
                    : gr.gridLayout.rowIntrinsicSizes;
                intrinsicResult = intrinsicSizes;
            }
        };

        auto normalPass = [&](const SizeResult& sr) {
            InlineSizingInput inlineSizing {
                .availableWidth = childConstraints.availableWidth,
                .widthRequest = sizeRequest.intrinsicWidthRequest,
                .trackIntrinsicWidth = sizeRequest.resolvingIntrinsicWidth,
            };
            
            // right now, minimum & maximum content dont really get set?
            // it only changes for flex/grid/etc...
            // which provide different contributions not based on intrinsic size collection but
            // min and max bounds; this needs to be fixed
            auto inlineFormatting = buildInlineBoxes(*this, node, frameInfo, childConstraints, sizeRequest, inlineSizing, sizeCache);

            if (inlineFormatting->intrinsicSizes) {
                intrinsicResult = *inlineFormatting->intrinsicSizes;
            }

            // if (node->id == 32) {
            //     std::println("red req: {} red h: {}", describeSizeState(sizeRequest.specified.height), describeSizeState(sr.outerSize.height));
            // }   

            // if (node->id == 33) {
            //     std::println("yellow req: {} yellow h: {}", describeSizeState(sizeRequest.specified.height), describeSizeState(sr.outerSize.height));
            // }   
            for (uint64_t i = 0; i < node->children.size(); ++i) {
                auto child = node->children[i].get();

                childConstraints.inlineFormatting = {
                    .context = inlineFormatting,
                    .fragments = inlineFormatting->childFragments[i],
                    .minFragments = inlineFormatting->minChildFragments.empty()
                        ? layout::InlineFragmentRange{}
                        : inlineFormatting->minChildFragments[i],
                    .maxFragments = inlineFormatting->maxChildFragments.empty()
                        ? layout::InlineFragmentRange{}
                        : inlineFormatting->maxChildFragments[i],
                };

                auto childOutput = layoutRecursive(child, frameInfo, childConstraints, mutate, std::nullopt, sizeRequest.intrinsicWidthRequest, sizeRequest.intrinsicHeightRequest);

                std::visit([&](const auto& childLayout) {
                if (!childLayout.outOfFlow) {
                    if (sizeRequest.resolvingIntrinsicWidth || sizeRequest.resolvingIntrinsicHeight) {
                        intrinsicResult->minimum = std::max(intrinsicResult->minimum, childOutput.intrinsicSizes->minimum);
                        intrinsicResult->maximum = std::max(intrinsicResult->maximum, childOutput.intrinsicSizes->maximum);
                    }

                    childConstraints.cursor = childLayout.siblingCursor;
                    childConstraints.edgeIntent = childLayout.edgeIntent;
                    childConstraints.prevInlineHeight = childLayout.prevInlineHeight;

                }
                }, childOutput.layout);
            }
        };

        auto display = node->getDisplay();

        switch (display) {
            case style::Display::Flex: {
                flexPass(sizeResult);
                break;
            }
            case style::Display::Grid: {
                gridPass(sizeResult);
                break;
            }
            default: {
                normalPass(sizeResult);
                break;
            }
        }

        std::visit([](auto& state) {
            state.localComputedBox = state.computedBox;
            state.localAtomOffsets = state.atomOffsets;
        }, layout);

        LayoutResult output {
            .layout = std::move(layout),
            .sizeResult = sizeResult,
            .intrinsicSizes = intrinsicResult
        };

        if (mutate) {
            node->layout = output;
            node->constraintsKey = key;
            node->dirtySelf |= DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }

        layoutCache[layoutKey] = output;

        return output;
    }

    void RenderTree::postLayoutPhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        auto key = makeConstraintsKey(constraints);
        auto reason = recomputeReason(node, DirtyBits::PostLayout, key);
        if (reason == instrumentation::RecomputeReason::None) {
            return;
        }
        instrumentation::recordRecompute(node->id, instrumentation::Phase::PostLayout, reason);

        auto& result = *node->layout;
        const auto& padding = result.sizeResult.padding;
        float paddingTop = std::holds_alternative<float>(padding.top) ? std::get<float>(padding.top) : 0.0f;
        float paddingRight = std::holds_alternative<float>(padding.right) ? std::get<float>(padding.right) : 0.0f;
        float paddingBottom = std::holds_alternative<float>(padding.bottom) ? std::get<float>(padding.bottom) : 0.0f;
        float paddingLeft = std::holds_alternative<float>(padding.left) ? std::get<float>(padding.left) : 0.0f;

        float borderWidth = std::holds_alternative<float>(result.sizeResult.borderWidth) ? std::get<float>(result.sizeResult.borderWidth) : 0.0f;

        const auto& outerSize = result.sizeResult.outerSize;
        float outerWidth = std::holds_alternative<float>(outerSize.width) ? std::get<float>(outerSize.width) : 0.0f;
        float outerHeight = std::holds_alternative<float>(outerSize.height) ? std::get<float>(outerSize.height) : 0.0f;
        
        const auto& paddingBoxSize = result.sizeResult.paddingBoxSize;
        float paddingBoxWidth = std::holds_alternative<float>(paddingBoxSize.width) ? std::max(0.0f, std::get<float>(paddingBoxSize.width)) : 0.0f;
        float paddingBoxHeight = std::holds_alternative<float>(paddingBoxSize.height) ? std::max(0.0f, std::get<float>(paddingBoxSize.height)) : 0.0f;


        auto position = node->getPosition();
        
        ContainingBlock containingBlock = constraints.containingBlock;
        if (position == Position::Fixed) {
            containingBlock = constraints.fixedContainingBlock;
        } else if (position == Position::Absolute) {
            containingBlock = constraints.absoluteContainingBlock;
        }

        std::visit([&](auto& layout) {
            // reset computed box to local variants
            layout.computedBox = layout.localComputedBox;
            layout.atomOffsets = layout.localAtomOffsets;

            // resolve right / bottom positioning
            // we only know how to position it after its size has been determiend
            if (layout.outOfFlow) {
                const auto& margins = node->preLayout->resolvedMargins;
                bool isRtl = constraints.inheritedProperties.direction == layout::Direction::rtl;

                std::optional<Size> rightInset = (!node->shared.left.has_value() || isRtl) ? node->shared.right : std::nullopt;
                std::optional<Size> bottomInset = !node->shared.top.has_value() ? node->shared.bottom : std::nullopt;

                if (rightInset) {
                    SizeState containingBlockWidth = calculateSize(containingBlock.width, std::monostate{});
                    SizeState right = calculateSize(*rightInset, containingBlock.width);
                    if (std::holds_alternative<float>(containingBlockWidth) && std::holds_alternative<float>(right)) {
                        float newX = std::get<float>(containingBlockWidth) - margins.right - outerWidth - std::get<float>(right);
                        float deltaX = newX - layout.computedBox.x;
                        layout.computedBox.x = newX;
                        for (auto& offset : layout.atomOffsets) {
                            offset.x += deltaX;
                        }
                    }
                }

                if (bottomInset) {
                    SizeState containingBlockHeight = calculateSize(containingBlock.height, std::monostate{});
                    SizeState bottom = calculateSize(*bottomInset, containingBlock.height);
                    if (std::holds_alternative<float>(containingBlockHeight) && std::holds_alternative<float>(bottom)) {
                        float newY = std::get<float>(containingBlockHeight) - margins.bottom - outerHeight - std::get<float>(bottom);
                        float deltaY = newY - layout.computedBox.y;
                        layout.computedBox.y = newY;
                        for (auto& offset : layout.atomOffsets) {
                            offset.y += deltaY;
                        }
                    }
                }
            }

            // establish post layout constraints
            simd_float2 baseOrigin = containingBlock.origin;

            layout.computedBox.x += baseOrigin.x;
            layout.computedBox.y += baseOrigin.y;
            for (auto& offset : layout.atomOffsets) {
                offset.x += baseOrigin.x;
                offset.y += baseOrigin.y;
            }

            node->globalOffset = baseOrigin; // why does this field matter?
            
            size_t clipCount = constraints.clipUniforms.size();
            if (position == Position::Absolute) {
                clipCount = constraints.absoluteContainingBlock.clipCount;
            } else if (position == Position::Fixed) {
                clipCount = constraints.fixedContainingBlock.clipCount;
            }
            layout.clipUniforms.assign(
                constraints.clipUniforms.begin(),
                constraints.clipUniforms.begin() + clipCount
            );

            // sticky adjustment
            if (position == Position::Sticky) {
                float stickyX = layout.computedBox.x;
                float stickyY = layout.computedBox.y;

                // sticky insets resolve against scrolport
                auto top = node->shared.top ? calculateSize(*node->shared.top, constraints.scrollport.height) : std::monostate{};
                auto bottom = node->shared.bottom ? calculateSize(*node->shared.bottom, constraints.scrollport.height) : std::monostate{};
                auto left = node->shared.left ? calculateSize(*node->shared.left, constraints.scrollport.width) : std::monostate{};
                auto right = node->shared.right ? calculateSize(*node->shared.right, constraints.scrollport.width) : std::monostate{};


                auto scrollportHeight = calculateSize(constraints.scrollport.height, std::monostate{});
                auto scrollportWidth = calculateSize(constraints.scrollport.width, std::monostate{});

                // determine sticky x/y after inset adjustment
                if (std::holds_alternative<float>(top)) {
                    auto resolvedTop = std::get<float>(top);
                    stickyY = std::max(stickyY, constraints.scrollport.origin.y + resolvedTop);
                }   

                if (std::holds_alternative<float>(bottom) && std::holds_alternative<float>(scrollportHeight)) {
                    auto resolvedBottom = std::get<float>(bottom);
                    auto resolvedScrollportHeight = std::get<float>(scrollportHeight);
                    stickyY = std::min(stickyY, constraints.scrollport.origin.y + resolvedScrollportHeight - resolvedBottom - outerHeight);
                }

                if (std::holds_alternative<float>(left)) {
                    auto resolvedLeft = std::get<float>(left);
                    stickyX = std::max(stickyX, constraints.scrollport.origin.x + resolvedLeft);
                }

                if (std::holds_alternative<float>(right) && std::holds_alternative<float>(scrollportWidth)) {
                    auto resolvedRight = std::get<float>(right);
                    auto resolvedScrollportWidth = std::get<float>(scrollportWidth);
                    stickyX = std::min(stickyX, constraints.scrollport.origin.x + resolvedScrollportWidth - resolvedRight - outerWidth);
                }

                // clamp inside containing block
                auto containingBlockWidth = calculateSize(containingBlock.width, std::monostate{});
                auto containingBlockHeight = calculateSize(containingBlock.height, std::monostate{});
                const auto& margins = node->preLayout->resolvedMargins;

                if (std::holds_alternative<float>(containingBlockWidth)) {
                    auto resolvedContainingBlockWidth = std::get<float>(containingBlockWidth);
                    auto minimumX = std::min(layout.computedBox.x, containingBlock.origin.x + margins.left);
                    auto maximumX = std::max(layout.computedBox.x, containingBlock.origin.x + resolvedContainingBlockWidth - margins.right - outerWidth);
                    stickyX = std::clamp(stickyX, minimumX, maximumX);
                }

                if (std::holds_alternative<float>(containingBlockHeight)) {
                    auto resolvedContainingBlockHeight = std::get<float>(containingBlockHeight);
                    auto minimumY = std::min(layout.computedBox.y, containingBlock.origin.y + margins.top);
                    auto maximumY = std::max(layout.computedBox.y, containingBlock.origin.y + resolvedContainingBlockHeight - margins.bottom - outerHeight);
                    stickyY = std::clamp(stickyY, minimumY, maximumY);
                }

                // adjust computed box + atom offsets
                float deltaX = stickyX - layout.computedBox.x;
                float deltaY = stickyY - layout.computedBox.y;
                layout.computedBox.x = stickyX;
                layout.computedBox.y = stickyY;
                for (auto& offset : layout.atomOffsets) {
                    offset.x += deltaX;
                    offset.y += deltaY;
                }
            }

            // figure out CSS transform properties
            // firstly; we determine the transform origin
            simd_float2 transformOrigin {
                layout.computedBox.x + node->shared.transformOriginX.resolveOr(Size::px(layout.computedBox.width)),
                layout.computedBox.y + node->shared.transformOriginY.resolveOr(Size::px(layout.computedBox.height))
            };

            simd_float3x3 toTransformOrigin = matrix_identity_float3x3;
            toTransformOrigin.columns[2] = {-transformOrigin.x, -transformOrigin.y, 1.0f};

            simd_float3x3 fromTransformOrigin = matrix_identity_float3x3;
            fromTransformOrigin.columns[2] = {transformOrigin.x, transformOrigin.y, 1.0f};

            // we do NOT want to move the actual shape; the goal of a CSS transform is explicitly to modify geomertry
            // so we subtract the origin (right mat-mult), apply the transform, then readd the origin (left mat-mult)
            simd_float3x3 localTransform = simd_mul(fromTransformOrigin, simd_mul(node->shared.transform.value_or(matrix_identity_float3x3), toTransformOrigin));
        
            node->transform = simd_mul(constraints.transform, localTransform);
            node->inverseTransform = simd_inverse(node->transform);
            node->effectiveOpacity = constraints.opacity * node->shared.opacity;

            node->atomized = node->element->postLayout(constraints, node->shared,*node->atomized, result.layout);

            // prepare child constraints; add clipping uniforms
            simd_float2 currPaddingOrigin = {
                layout.computedBox.x + borderWidth,
                layout.computedBox.y + borderWidth
            };


            simd_float2 currContentOrigin = {
                layout.computedBox.x + borderWidth + paddingLeft,
                layout.computedBox.y + borderWidth + paddingTop
            };

            ContainingBlock childScrollport = constraints.scrollport;

            if (node->shared.overflow == Overflow::Scroll) {
                // compute scroll port size
                node->scrollViewportSize = {
                    paddingBoxWidth,
                    paddingBoxHeight
                };

                childScrollport = {
                    .origin = currPaddingOrigin,
                    .width = paddingBoxSize.width,
                    .height = paddingBoxSize.width
                };

                // adjust origins by scroll offsets
                float scrollX = constraints.inheritedProperties.direction == layout::Direction::rtl ? node->scrollOffset.x : -node->scrollOffset.x;

                currContentOrigin.x += scrollX;
                currContentOrigin.y -= node->scrollOffset.y;
                currPaddingOrigin.x += scrollX;
                currPaddingOrigin.y -= node->scrollOffset.y;
            }

            auto childConstraints = constraints;
            childConstraints.clipUniforms = layout.clipUniforms;
            childConstraints.availableWidth = layout.childConstraints.availableWidth;
            childConstraints.availableHeight = layout.childConstraints.availableHeight;
            childConstraints.transform = node->transform;
            childConstraints.opacity = node->effectiveOpacity;

            childConstraints.containingBlock = {
                .origin = currContentOrigin,
                .width = result.sizeResult.innerSize.width,
                .height = result.sizeResult.innerSize.height
            };

            childConstraints.scrollport = childScrollport;

            if (node->shared.overflow != Overflow::Visible) {
                childConstraints.textOverflow = node->shared.textOverflow;

                simd_float2 halfExtent {
                    paddingBoxWidth * 0.5f,
                    paddingBoxHeight * 0.5f
                };

                childConstraints.clipUniforms.push_back({
                    .rectCenter = {
                        layout.computedBox.x + halfExtent.x,
                        layout.computedBox.y + halfExtent.y
                    },
                    .halfExtent = halfExtent,
                    .cornerRadius = style::resolveCornerRadii(
                        node->shared,
                        outerWidth,
                        outerHeight
                    ),
                    .inverseTransform = node->inverseTransform
                });
            }

            if (position != Position::Static || node->shared.transform) {
                childConstraints.absoluteContainingBlock = {
                    .origin = currPaddingOrigin,
                    .width = paddingBoxSize.width,
                    .height = paddingBoxSize.height,
                    .clipCount = childConstraints.clipUniforms.size()
                };
            }

            if (node->shared.transform) {
                childConstraints.fixedContainingBlock = childConstraints.absoluteContainingBlock;
            }

            // recurse
            for (auto& child : node->children) {
                postLayoutPhase(child.get(), frameInfo, childConstraints);
            }


            // measure scrollable content and update scroll extent
            if (node->shared.overflow == Overflow::Scroll) {
                simd_float2 contentSize {0.0f, 0.0f};
                std::function<void(TreeNode*, bool, bool, simd_float3x3)> includeChildOverflow;
                includeChildOverflow = [&](TreeNode* child, bool absoluteContainingBlockEstablished, bool fixedContainingBlockEstablished, simd_float3x3 parentTransform) {
                    std::visit([&](const auto& childLayout) {
                        // the overflow rules are a mess, but basically:
                        // if we have an absolute containing block established within the subtree starting with the overflowing container
                        // at the root; then recurse; else stop.
                        if (child->shared.position == Position::Absolute && !absoluteContainingBlockEstablished) {
                            return;
                        }
                        if (child->shared.position == Position::Fixed && !fixedContainingBlockEstablished) {
                            return;
                        }

                        const auto& childBox = childLayout.computedBox;
                        const auto& childOuterSize = child->layout->sizeResult.outerSize;
                        float childOuterWidth = std::holds_alternative<float>(childOuterSize.width) ? std::get<float>(childOuterSize.width) : 0.0f;
                        float childOuterHeight = std::holds_alternative<float>(childOuterSize.height) ? std::get<float>(childOuterSize.height) : 0.0f;

                        // here; we have to include overflow of transforms
                        simd_float2 transformOrigin {
                            childBox.x + child->shared.transformOriginX.resolveOr(Size::px(childBox.width)),
                            childBox.y + child->shared.transformOriginY.resolveOr(Size::px(childBox.height))
                        };

                        simd_float3x3 toTransformOrigin = matrix_identity_float3x3;
                        toTransformOrigin.columns[2] = {-transformOrigin.x, -transformOrigin.y, 1.0f};

                        simd_float3x3 fromTransformOrigin = matrix_identity_float3x3;
                        fromTransformOrigin.columns[2] = {transformOrigin.x, transformOrigin.y, 1.0f};

                        simd_float3x3 localTransform = simd_mul(fromTransformOrigin, simd_mul(child->shared.transform.value_or(matrix_identity_float3x3), toTransformOrigin));
                        simd_float3x3 childTransform = simd_mul(parentTransform, localTransform);

                        float left = childBox.x;
                        float right = childBox.x + childOuterWidth;
                        float bottom = childBox.y + childOuterHeight;
                        simd_float3 transformedTopLeft = simd_mul(childTransform, simd_float3{left, childBox.y, 1.0f});
                        simd_float3 transformedTopRight = simd_mul(childTransform, simd_float3{right, childBox.y, 1.0f});
                        simd_float3 transformedBottomLeft = simd_mul(childTransform, simd_float3{left, bottom, 1.0f});
                        simd_float3 transformedBottomRight = simd_mul(childTransform, simd_float3{right, bottom, 1.0f});

                        // our scroll container must not *shrink*; thus, we take mins/max's for left/right/bot
                        left = std::min({left, transformedTopLeft.x, transformedTopRight.x, transformedBottomLeft.x, transformedBottomRight.x});
                        right = std::max({right, transformedTopLeft.x, transformedTopRight.x, transformedBottomLeft.x, transformedBottomRight.x});
                        bottom = std::max({bottom, transformedTopLeft.y, transformedTopRight.y, transformedBottomLeft.y, transformedBottomRight.y});

                        if (constraints.inheritedProperties.direction == layout::Direction::rtl) {
                            // rtl needs padding subtracted bc it starts from the RIGHT edge, (which includes both paddings)
                            contentSize.x = std::max(
                                contentSize.x,
                                currContentOrigin.x
                                    + node->scrollViewportSize.x
                                    - paddingLeft
                                    - paddingRight
                                    - left
                            );
                        } else {
                            contentSize.x = std::max(contentSize.x, right - currContentOrigin.x);
                        }

                        contentSize.y = std::max(contentSize.y, bottom - currContentOrigin.y);

                        if (child->shared.overflow != Overflow::Visible){
                            return;
                        }
                        
                        for (auto& grandchild : child->children) {
                            includeChildOverflow(
                                grandchild.get(),
                                absoluteContainingBlockEstablished || child->shared.position != Position::Static || child->shared.transform,
                                fixedContainingBlockEstablished || child->shared.transform,
                                childTransform
                            );
                        }
                    }, child->layout->layout);
                };

                bool absoluteContainingBlockEstablished = position != Position::Static || node->shared.transform;
                bool fixedContainingBlockEstablished = node->shared.transform.has_value();
                for (auto& child : node->children) {
                    includeChildOverflow(child.get(), absoluteContainingBlockEstablished, fixedContainingBlockEstablished, matrix_identity_float3x3);
                }

                // need to include padding after again so content size is correct
                node->scrollContentSize = {
                    std::max(
                        node->scrollViewportSize.x,
                        contentSize.x + paddingLeft + paddingRight
                    ),
                    std::max(
                        node->scrollViewportSize.y,
                        contentSize.y + paddingTop + paddingBottom
                    )
                };
            }
        }, result.layout);

        node->constraintsKey = key;
        node->dirtySelf |= DirtyBits::Place | DirtyBits::Finalize;
    }
    
    void RenderTree::placePhase(TreeNode* node, const FrameInfo& frameInfo, Constraints& constraints) {
        auto key = makeConstraintsKey(constraints);
        auto reason = recomputeReason(node, DirtyBits::Place, key);
        if (reason != instrumentation::RecomputeReason::None) {
            instrumentation::recordRecompute(node->id, instrumentation::Phase::Place, reason);
            auto& atomized = *node->atomized;
            auto& layout = node->layout->layout;

            auto placed = node->element->place(constraints, node->shared, atomized, layout);
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
            auto& atomized = *node->atomized;
            auto& layout = node->layout->layout;
            auto& placed = *node->placed;
            auto finalizedConstraints = constraints;
            finalizedConstraints.transform = node->transform;
            finalizedConstraints.opacity = node->effectiveOpacity;
            auto finalized = node->element->finalize(finalizedConstraints, node->shared, atomized, layout, placed);
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
                auto isInSubtree = node->paintPreorderIndex <= candidate->paintPreorderIndex && candidate->paintPostorderIndex <= node->paintPostorderIndex;
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
