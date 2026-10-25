#include "tree_node.hpp"
#include "fragment_types.hpp"
#include "layout/sizing.hpp"
#include "layout/layout.hpp"
#include "render_tree.hpp"
#include "context_manager.hpp"
#include "overloaded.hpp"
#include "utf8.hpp"
#include "text/textShaper.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <simd/vector_types.h>
#include <string>
#include <unordered_map>
#include <utility>


namespace tree {
    using elements::DescriptorPayload;
    using elements::GetField;
    using elements::RequestTarget;
    using elements::isTextWhitespace;
    using layout::Constraints;
    using layout::LayoutEngine;
    using layout::LayoutInput;
    using layout::LineBox;
    using layout::LineFragment;
    using style::Display;
    using style::Position;
    using style::Size;
    using style::WhiteSpace;
    using style::WordBreak;

    // Element-specific requests still use the request system
    std::optional<std::string> getText(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "text"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<std::string>(resp);
        }
        return std::nullopt;
    }

    void appendInlineFlow(TreeNode* node, std::vector<InlineFlowItem>& flow) {
        for (auto& child : node->children) {
            if (getText(child.get()).has_value()) {
                flow.push_back({.kind = InlineFlowKind::Text, .node = child.get()});
            } else if (child->element->isInline()) {
                flow.push_back({.kind = InlineFlowKind::Atomic, .node = child.get()});
            } else if (child->getDisplay() == Display::Inline) {
                flow.push_back({.kind = InlineFlowKind::Open, .node = child.get()});
                if (child->children.empty()) {
                    flow.push_back({.kind = InlineFlowKind::Placeholder, .node = child.get()});
                } else {
                    appendInlineFlow(child.get(), flow);
                }
                flow.push_back({.kind = InlineFlowKind::Close, .node = child.get()});
            } else if (node->getDisplay() == Display::Inline) {
                // in the <open><block><close> case 
                // open and close live on diff lines
                // but there wasnt a thing for the open placeholder to attach to for edges/reordering
                // so placeholder fixes this
                flow.push_back({.kind = InlineFlowKind::Placeholder, .node = node});
                flow.push_back({.kind = InlineFlowKind::Block, .node = child.get()});
                flow.push_back({.kind = InlineFlowKind::Placeholder, .node = node});
            } else {
                flow.push_back({.kind = InlineFlowKind::Block, .node = child.get()});
            }
        }
    }

    std::vector<InlineFlowItem> inlineFlow(TreeNode* container) {
        std::vector<InlineFlowItem> flow;
        flow.reserve(container->children.size());
        appendInlineFlow(container, flow);
        return flow;
    }

    void appendBidiText(TreeNode* node, std::string& paragraph) {
        if (auto text = getText(node)) {
            std::string bidiText = *text;
            const auto whiteSpace = getWhiteSpace(node)
                .value_or(WhiteSpace::Normal);
            if (whiteSpace == WhiteSpace::Normal || whiteSpace == WhiteSpace::NoWrap) {
                for (char& byte : bidiText) {
                    if (byte == '\r' || byte == '\n') byte = ' ';
                }
            }

            paragraph += bidiText;
        } else if (node->element->isInline()) {
            paragraph += "\xEF\xBF\xBC";
        } else if (node->getDisplay() == Display::Inline) {
            for (auto& child : node->children) {
                appendBidiText(child.get(), paragraph);
            }
        } else {
            // block handling; splits the line
            paragraph += "\n";
        }
    }

    bidi::TextBidiInput sliceTextBidiInput(
        const std::shared_ptr<const bidi::TextBidiContext>& context,
        size_t sliceStart,
        size_t sliceLength
    ) {
        const size_t sliceEnd = sliceStart + sliceLength;
        std::vector<bidi::TextShapingRun> sliceRuns;
        for (const auto& run : context->runs()) {
            const size_t runEnd = run.byteStart + run.byteLength;
            const size_t start = std::max(sliceStart, run.byteStart);
            const size_t end = std::min(sliceEnd, runEnd);
            if (start >= end) {
                continue;
            }
            sliceRuns.push_back({
                .byteStart = start - sliceStart,
                .byteLength = end - start,
                .level = run.level,
                .scriptTag = run.scriptTag
            });
        }
        return bidi::TextBidiInput{
            .context = context,
            .paragraphByteStart = sliceStart,
            .byteLength = sliceLength,
            .runs = sliceRuns
        };
    }

    Result<std::vector<std::optional<bidi::TextBidiInput>>> prepareChildBidiInputs(
        TreeNode* parent,
        layout::Direction baseDirection
    ) {
        std::vector<std::optional<bidi::TextBidiInput>> inputs(parent->children.size());

        // avoid allocs if we're a leaf node
        if (parent->children.empty()) {
            return inputs;
        }

        // the child will split the parent text accordingly; we hand the children the slice range
        // this is for proper recursive handling now that we have inline children
        bidi::TextBidiInput source;
        if (parent->getDisplay() == Display::Inline) {
            source = *parent->textBidiInput;
        } else {
            std::string paragraph;
            for (auto& child : parent->children) {
                appendBidiText(child.get(), paragraph);
            }

            auto resolvedContext = bidi::TextBidiContext::create(
                paragraph,
                baseDirection == layout::Direction::rtl
                    ? bidi::BidiBaseDirection::Rtl
                    : bidi::BidiBaseDirection::Ltr
            );
            if (!resolvedContext) return std::unexpected{resolvedContext.error()};
            source = bidi::TextBidiInput{
                .context = std::make_shared<bidi::TextBidiContext>(std::move(*resolvedContext)),
                .paragraphByteStart = 0,
                .byteLength = paragraph.size()
            };
        }

        size_t childStart = source.paragraphByteStart;
        for (size_t i = 0; i < parent->children.size(); ++i) {
            std::string childText;
            appendBidiText(parent->children[i].get(), childText);
            inputs[i] = sliceTextBidiInput(source.context, childStart, childText.size());
            childStart += childText.size();
        }

        return inputs;
    }

    std::optional<WhiteSpace> getWhiteSpace(TreeNode* node) {
        return node->inheritedProperties.whiteSpace;
    }

    std::optional<WordBreak> getWordBreak(TreeNode* node) {
        return node->inheritedProperties.wordBreak;
    }

    ShapedRun* getShapedRun(TreeNode* node) {
        std::any request;
        auto response = node->element->request(RequestTarget::TextShaping, request);
        return std::any_cast<ShapedRun*>(response);
    }
    

    void pushRunFragments(
        const ShapedRun& shapedRun,
        const std::vector<Atom>& atoms,
        size_t clusterStart,
        size_t clusterEnd,
        float totalWidth,
        std::vector<LineFragment>& fragments
    ) {
        if (clusterStart == clusterEnd) {
            return;
        }

        float glyphWidth = 0.0f;
        for (size_t i = clusterStart; i < clusterEnd; ++i) {
            const auto& cluster = shapedRun.clusters[i];
            for (size_t glyph = 0; glyph < cluster.glyphCount; ++glyph) {
                glyphWidth += atoms[cluster.glyphStart + glyph].width;
            }
        }
        const float trailingWidth = totalWidth - glyphWidth;
        const auto& finalCluster = shapedRun.clusters[clusterEnd - 1];
        const size_t finalByteEnd = finalCluster.byteOffset + finalCluster.byteLength;

        // here we basically search for where this specific run (a bidi run) intersects with the cluster
        // range we specified
        for (const auto& run : shapedRun.runs) {
            const size_t runStart = std::max(clusterStart, run.clusterStart);
            const size_t runEnd = std::min(clusterEnd, run.clusterStart + run.clusterCount);

            if (runStart >= runEnd) {
                continue;
            }

            size_t atomStart = atoms.size();
            size_t atomEnd = 0;
            float width = 0.0f;
            float height = 0.0f;
            float ascent = 0.0f;
            float descent = 0.0f;

            for (size_t i = runStart; i < runEnd; ++i) {
                const auto& cluster = shapedRun.clusters[i];
                atomStart = std::min(atomStart, cluster.glyphStart);
                atomEnd = std::max(atomEnd, cluster.glyphStart + cluster.glyphCount);
                for (size_t glyph = 0; glyph < cluster.glyphCount; ++glyph) {
                    const auto& atom = atoms[cluster.glyphStart + glyph];
                    width += atom.width;
                    height = std::max(height, atom.lineHeight);
                    ascent = std::max(ascent, atom.ascent);
                    descent = std::max(descent, atom.descent);
                }
            }

            const auto& firstCluster = shapedRun.clusters[runStart];
            const auto& lastCluster = shapedRun.clusters[runEnd - 1];
            const size_t byteStart = firstCluster.byteOffset;
            const size_t byteEnd = lastCluster.byteOffset + lastCluster.byteLength;

            if (byteEnd == finalByteEnd) {
                width += trailingWidth;
            }

            LineFragment fragment{
                .kind = InlineFlowKind::Text,
                .width = width,
                .height = height,
                .ascent = ascent,
                .descent = descent,
                .atomStart = atomStart,
                .atomCount = atomEnd - atomStart,
                .textByteStart = byteStart,
                .textByteLength = byteEnd - byteStart,
                .bidiLevel = layout::ResolvedLevel{run.bidiLevel},
            };

            fragments.push_back(fragment);
        }
    }

    void reorderLineFragments(layout::InlineFormattingContext& context, bool isLtr) {
        std::vector<LineFragment*> fragments;
        fragments.reserve(context.fragments.size());
        std::vector<uint8_t> levels;
        levels.reserve(context.fragments.size());
        struct LineElementSlice {
            size_t start = 0;
            size_t end = 0;
            size_t first = std::numeric_limits<size_t>::max();
            size_t last = 0;
        };
        std::unordered_map<size_t, LineElementSlice> lineElementSlices;

        std::vector<LineFragment*> visual;
        visual.reserve(context.fragments.size());

        for (size_t lineIndex = 0; lineIndex < context.lineBoxes.size(); ++lineIndex) {
            auto& lineBox = context.lineBoxes[lineIndex];
            fragments.clear();
            levels.clear();
            int maximumLevel = 0;
            int minimumOddLevel = -1;

            for (size_t i = 0; i < lineBox.fragmentCount; ++i) {
                auto& fragment = context.fragments[lineBox.fragmentStart + i];
                if (fragment.bidiLevel) {
                    fragments.push_back(&fragment);
                }
            }

            levels.resize(fragments.size());
            uint8_t nextLevel = isLtr ? 0 : 1;
            for (size_t i = fragments.size(); i-- > 0;) {
                nextLevel = std::visit(Overloaded{
                    [](const layout::ResolvedLevel& resolved) { return resolved.level; },
                    [&](const layout::InheritedLevel&) { return nextLevel; }
                }, *fragments[i]->bidiLevel);
                levels[i] = nextLevel;
            }

            for (uint8_t level : levels) {
                maximumLevel = std::max(maximumLevel, static_cast<int>(level));
                if ((level & 1u) != 0 && (minimumOddLevel == -1 || level < minimumOddLevel)) {
                    minimumOddLevel = level;
                }
            }

            if (minimumOddLevel != -1) {
                for (int level = maximumLevel; level >= minimumOddLevel; --level) {
                    size_t start = 0;
                    while (start < fragments.size()) {
                        while (start < fragments.size() &&
                               levels[start] < level) {
                            ++start;
                        }

                        size_t end = start;
                        while (end < fragments.size() &&
                               levels[end] >= level) {
                            ++end;
                        }

                        std::reverse(
                            fragments.begin() + start,
                            fragments.begin() + end
                        );
                        std::reverse(
                            levels.begin() + start,
                            levels.begin() + end
                        );
                        start = end;
                    }
                }
            }

            auto* lineStart = &context.fragments[lineBox.fragmentStart];
            lineElementSlices.clear();

            for (size_t i = 0; i < lineBox.fragmentCount; ++i) {
                auto& fragment = lineStart[i];
                if (fragment.kind == InlineFlowKind::Open) {
                    auto& slice = lineElementSlices.try_emplace(fragment.elementIndex, LineElementSlice{.end = lineBox.fragmentCount}).first->second;
                    slice.start = i + 1;
                } else if (fragment.kind == InlineFlowKind::Close) {
                    auto& slice = lineElementSlices.try_emplace(fragment.elementIndex, LineElementSlice{.end = lineBox.fragmentCount}).first->second;
                    slice.end = i;
                }
            }

            for (size_t v = 0; v < fragments.size(); ++v) {
                const size_t logical = fragments[v] - lineStart;
                for (auto& [elementIndex, slice] : lineElementSlices) {
                    if (logical >= slice.start && logical < slice.end) {
                        slice.first = std::min(slice.first, v);
                        slice.last = std::max(slice.last, v);
                    }
                }
            }

            visual.clear();

            for (size_t v = 0; v < fragments.size(); ++v) {
                for (size_t i = 0; i < lineBox.fragmentCount; ++i) {
                    auto& edge = lineStart[isLtr ? i : lineBox.fragmentCount - 1 - i];
                    if (edge.kind != InlineFlowKind::Open && edge.kind != InlineFlowKind::Close) {
                        continue;
                    }

                    if ((edge.kind == InlineFlowKind::Open) == isLtr && lineElementSlices[edge.elementIndex].first == v) {
                        visual.push_back(&edge);
                    }
                }

                visual.push_back(fragments[v]);

                for (size_t i = 0; i < lineBox.fragmentCount; ++i) {
                    auto& edge = lineStart[isLtr ? i : lineBox.fragmentCount - 1 - i];
                    if (edge.kind != InlineFlowKind::Open && edge.kind != InlineFlowKind::Close) {
                        continue;
                    }

                    if ((edge.kind == InlineFlowKind::Open) != isLtr && lineElementSlices[edge.elementIndex].last == v) {
                        visual.push_back(&edge);
                    }
                }
            }

            float offset = 0.0f;

            for (auto* fragment : visual) {
                fragment->offset = isLtr ? offset + fragment->leadingMargin : offset + fragment->trailingMargin;
                offset += fragment->leadingMargin + fragment->width + fragment->trailingMargin;
            }
        }
    }

    bool shouldTakeSoftBreak(
        std::optional<IntrinsicRequest> widthRequest,
        bool lineHasContent,
        float prospectiveWidth,
        const SizeState& availableWidth
    ) {
        if (!lineHasContent) {
            return false;
        }

        if (widthRequest == IntrinsicRequest::Minimum) {
            return true;
        }

        if (widthRequest == IntrinsicRequest::Maximum) {
            return false;
        }

        const auto* resolvedAvailableWidth = std::get_if<float>(&availableWidth);
        return resolvedAvailableWidth && prospectiveWidth > *resolvedAvailableWidth;
    }

    void commitPendingFragments(
        std::optional<IntrinsicRequest> widthRequest,
        const SizeState& availableWidth,
        std::vector<LineFragment>& fragments,
        std::vector<LineBox>& lineBoxes,
        LineBox& currentLineBox,
        size_t& currentLineBoxIndex,
        size_t& pendingStart,
        float& pendingWidth
    ) {
        if (pendingStart == fragments.size()) {
            return;
        }

        if (shouldTakeSoftBreak(
                widthRequest,
                currentLineBox.fragmentCount > 0,
                currentLineBox.width + pendingWidth,
                availableWidth
            )) {
            lineBoxes.push_back(std::move(currentLineBox));
            currentLineBox = {};
            currentLineBoxIndex++;
        }

        if (currentLineBox.fragmentCount == 0) {
            currentLineBox.fragmentStart = pendingStart;
        }

        for (size_t f = pendingStart; f < fragments.size(); ++f) {
            const auto& fragment = fragments[f];
            const float halfLeading = (fragment.height - fragment.ascent - fragment.descent) / 2.0f;
            fragments[f].lineBoxIndex = currentLineBoxIndex;
            currentLineBox.ascent = std::max(currentLineBox.ascent, fragment.ascent + halfLeading);
            currentLineBox.descent = std::max(currentLineBox.descent, fragment.descent + halfLeading);
        }
        currentLineBox.height = currentLineBox.ascent + currentLineBox.descent;

        currentLineBox.fragmentCount += fragments.size() - pendingStart;
        currentLineBox.width += pendingWidth;
        pendingStart = fragments.size();
        pendingWidth = 0.0f;
    }

    void appendAtomicInlineFragment(
        std::vector<Atom>& atoms,
        const SizeResult& sizeResult,
        const bidi::TextShapingRun& run,
        ResolvedMargins margins,
        const SizeState& availableWidth,
        std::optional<IntrinsicRequest> widthRequest,
        std::vector<LineFragment>& fragments,
        std::vector<LineBox>& lineBoxes,
        LineBox& currentLineBox,
        size_t& currentLineBoxIndex,
        size_t& pendingStart,
        float& pendingWidth,
        bool& lastFragmentHasBreakOpportunity
    ) {
        if (atoms.empty()) {
            return;
        }

        if (std::holds_alternative<float>(sizeResult.borderBoxSize.width)) {
            atoms.front().width = std::get<float>(sizeResult.borderBoxSize.width);
        }

        if (std::holds_alternative<float>(sizeResult.borderBoxSize.height)) {
            atoms.front().height = std::get<float>(sizeResult.borderBoxSize.height);
        }
        atoms.front().ascent = atoms.front().height;

        commitPendingFragments(
            widthRequest,
            availableWidth,
            fragments,
            lineBoxes,
            currentLineBox,
            currentLineBoxIndex,
            pendingStart,
            pendingWidth
        );

        LineFragment fragment {
            .kind = InlineFlowKind::Atomic,
            .width = atoms.front().width,
            .height = atoms.front().height,
            .ascent = atoms.front().ascent,
            .atomStart = 0,
            .atomCount = atoms.size(),
            .bidiLevel = layout::ResolvedLevel{run.level},
        };

        fragments.push_back(fragment);
        pendingWidth += margins.left + margins.right + atoms.front().width;
        lastFragmentHasBreakOpportunity = true;
    }

    void appendTextLineFragments(
        const std::string& text,
        const ShapedRun& shapedRun,
        const std::vector<Atom>& atoms,
        WhiteSpace whiteSpace,
        WordBreak wordBreak,
        ResolvedMargins margins,
        bool isLtr,
        const SizeState& availableWidth,
        std::optional<IntrinsicRequest> widthRequest,
        std::vector<LineFragment>& fragments,
        std::vector<LineBox>& lineBoxes,
        LineBox& currentLineBox,
        size_t& currentLineBoxIndex,
        size_t& pendingStart,
        float& pendingWidth,
        bool& lastFragmentHasBreakOpportunity
    ) {
        const float marginStart = isLtr ? margins.left : margins.right;
        const float marginEnd = isLtr ? margins.right : margins.left;

        const bool preserveLineFeeds = whiteSpace == WhiteSpace::Pre ||  whiteSpace == WhiteSpace::PreWrap;
        const bool allowSoftWrap = whiteSpace == WhiteSpace::Normal || whiteSpace == WhiteSpace::PreWrap;
        const bool breakInsideWords = allowSoftWrap && wordBreak == WordBreak::BreakAll;

        float runningWidth = marginStart;
        float runningEdge = marginStart;

        size_t runningAtomCount = 0;
        size_t runningClusterStart = 0;
        size_t idx = 0;

        while (idx < shapedRun.clusters.size()) {
            const auto& cluster = shapedRun.clusters[idx];
            char32_t ch = cluster.leadCodepoint;
            const auto& firstAtom = atoms[cluster.glyphStart];
            float width = 0.0f;
            for (size_t i = 0; i < cluster.glyphCount; ++i) {
                width += atoms[cluster.glyphStart + i].width;
            }

            if (preserveLineFeeds && firstAtom.placeOnNewLine) {
                runningWidth += width;

                if (idx + 1 >= shapedRun.clusters.size()) {
                    runningWidth += marginEnd;
                    runningEdge += marginEnd;
                }

                runningAtomCount += cluster.glyphCount;

                if (lastFragmentHasBreakOpportunity) {
                    commitPendingFragments(
                        widthRequest,
                        availableWidth,
                        fragments,
                        lineBoxes,
                        currentLineBox,
                        currentLineBoxIndex,
                        pendingStart,
                        pendingWidth
                    );
                }

                pushRunFragments(
                    shapedRun,
                    atoms,
                    runningClusterStart,
                    idx + 1,
                    runningWidth - runningEdge,
                    fragments
                );
                pendingWidth += runningWidth;

                commitPendingFragments(
                    widthRequest,
                    availableWidth,
                    fragments,
                    lineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    pendingStart,
                    pendingWidth
                );

                lineBoxes.push_back(std::move(currentLineBox));
                currentLineBox = {};
                currentLineBoxIndex++;
                lastFragmentHasBreakOpportunity = false;

                runningWidth = 0.0;
                runningEdge = 0.0;
                runningAtomCount = 0;
                idx++;
                runningClusterStart = idx;
                continue;
            }

            if (breakInsideWords && !isTextWhitespace(ch)) {
                runningWidth += width;

                if (idx + 1 >= shapedRun.clusters.size()) {
                    runningWidth += marginEnd;
                    runningEdge += marginEnd;
                }

                if (lastFragmentHasBreakOpportunity) {
                    commitPendingFragments(
                        widthRequest,
                        availableWidth,
                        fragments,
                        lineBoxes,
                        currentLineBox,
                        currentLineBoxIndex,
                        pendingStart,
                        pendingWidth
                    );
                }

                pushRunFragments(
                    shapedRun,
                    atoms,
                    runningClusterStart,
                    idx + 1,
                    runningWidth - runningEdge,
                    fragments
                );
                pendingWidth += runningWidth;
                lastFragmentHasBreakOpportunity = true;

                runningWidth = 0.0f;
                runningEdge = 0.0f;
                runningAtomCount = 0;
                idx++;
                runningClusterStart = idx;
                continue;
            }

            if (!isTextWhitespace(ch)) {
                runningWidth += width;
                runningAtomCount += cluster.glyphCount;
                idx++;
                continue;
            }

            while (idx < shapedRun.clusters.size() &&
                   isTextWhitespace(shapedRun.clusters[idx].leadCodepoint)) {
                const auto& whitespace = shapedRun.clusters[idx];
                for (size_t i = 0; i < whitespace.glyphCount; ++i) {
                    runningWidth += atoms[whitespace.glyphStart + i].width;
                }
                runningAtomCount += whitespace.glyphCount;
                idx++;
            }

            if (idx >= shapedRun.clusters.size()) {
                runningWidth += marginEnd;
                runningEdge += marginEnd;
            }

            if (lastFragmentHasBreakOpportunity) {
                commitPendingFragments(
                    widthRequest,
                    availableWidth,
                    fragments,
                    lineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    pendingStart,
                    pendingWidth
                );
            }

            pushRunFragments(
                shapedRun,
                atoms,
                runningClusterStart,
                idx,
                runningWidth - runningEdge,
                fragments
            );
            pendingWidth += runningWidth;
            lastFragmentHasBreakOpportunity = allowSoftWrap;

            runningWidth = 0.0;
            runningEdge = 0.0;
            runningAtomCount = 0;
            runningClusterStart = idx;
        }

        if (runningAtomCount > 0) {
            runningWidth += marginEnd;
            runningEdge += marginEnd;

            if (lastFragmentHasBreakOpportunity) {
                commitPendingFragments(
                    widthRequest,
                    availableWidth,
                    fragments,
                    lineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    pendingStart,
                    pendingWidth
                );
            }

            pushRunFragments(
                shapedRun,
                atoms,
                runningClusterStart,
                shapedRun.clusters.size(),
                runningWidth - runningEdge,
                fragments
            );
            pendingWidth += runningWidth;
            lastFragmentHasBreakOpportunity = false;
        }
    }
    

    std::shared_ptr<layout::InlineFormattingContext> buildInlineBoxes(
        RenderTree& tree, TreeNode* node, const FrameInfo& frameInfo,
        const Constraints& constraints, const SizeRequest& request, const InlineSizingInput& sizing,
        std::optional<std::unordered_map<size_t, SizeResult>&> sizeCache) {
        auto context = std::make_shared<layout::InlineFormattingContext>();
        auto& childrenLineBoxes = context->lineBoxes;
        auto& fragments = context->fragments;
        auto& nodeFragments = context->nodeFragments;
        LineBox currentLineBox {};
        size_t currentLineBoxIndex = 0;
        size_t pendingStart = 0;
        float pendingWidth = 0.0f;
        bool lastFragmentHasBreakOpportunity = false;
        SizeState availableWidth = calculateSize(sizing.availableWidth, std::monostate{});
        const bool isLtr = constraints.inheritedProperties.direction == layout::Direction::ltr;


        const auto flow = inlineFlow(node);

        for (size_t i = 0; i < flow.size(); ++i) {
            auto* child = flow[i].node;

            size_t childFragmentStart = fragments.size();

            if (flow[i].kind != InlineFlowKind::Block) {
                auto textResp = getText(child);
                auto margins = child->preLayout->resolvedMargins;
                auto& atoms = child->atomized->atoms;

                SizeRequest childRequest {
                    .position = child->shared.position,
                    .specified = {
                        .width = tree.animationManager.width(child->animations[animation::Animatable::Width], child->getWidth()),
                        .height = tree.animationManager.height(child->animations[animation::Animatable::Height], child->getHeight()),
                    },
                    .minimum = {.width = child->shared.minWidth, .height = child->shared.minHeight},
                    .maximum = {
                        .width = child->shared.maxWidth ? SizeState{*child->shared.maxWidth} : SizeState{std::monostate{}},
                        .height = child->shared.maxHeight ? SizeState{*child->shared.maxHeight} : SizeState{std::monostate{}},
                    },
                    .available = {.width = constraints.availableWidth, .height = constraints.availableHeight},
                    .top = child->shared.top,
                    .right = child->shared.right,
                    .bottom = child->shared.bottom,
                    .left = child->shared.left,
                    .paddingTop = child->shared.paddingTop.value_or(child->shared.padding),
                    .paddingRight = child->shared.paddingRight.value_or(child->shared.padding),
                    .paddingBottom = child->shared.paddingBottom.value_or(child->shared.padding),
                    .paddingLeft = child->shared.paddingLeft.value_or(child->shared.padding),
                    .borderTop = child->shared.border.topWidth.value_or(child->shared.border.width),
                    .borderRight = child->shared.border.rightWidth.value_or(child->shared.border.width),
                    .borderBottom = child->shared.border.bottomWidth.value_or(child->shared.border.width),
                    .borderLeft = child->shared.border.leftWidth.value_or(child->shared.border.width),
                    .margins = margins,
                    .aspectRatio = child->shared.aspectRatio,
                    .automaticWidth = (child->getPosition() == Position::Absolute || child->getPosition() == Position::Fixed)
                        ? AutomaticSizing::UseContent : AutomaticSizing::UseAvailable,
                    .automaticHeight = AutomaticSizing::UseContent,
                    .automaticMinimumWidth = AutomaticMinimum::Zero,
                    .automaticMinimumHeight = AutomaticMinimum::Zero,
                    .intrinsicWidthRequest = request.intrinsicWidthRequest,
                    .intrinsicHeightRequest = request.intrinsicHeightRequest,
                    .resolvingIntrinsicWidth = request.intrinsicWidthRequest.value_or(IntrinsicRequest::None) != IntrinsicRequest::None,
                    .resolvingIntrinsicHeight = request.intrinsicHeightRequest.value_or(IntrinsicRequest::None) != IntrinsicRequest::None,
                };
                if (flow[i].kind == InlineFlowKind::Text) {
                    appendTextLineFragments(
                        *textResp,
                        *getShapedRun(child),
                        atoms,
                        getWhiteSpace(child).value_or(WhiteSpace::Normal),
                        getWordBreak(child).value_or(WordBreak::Normal),
                        ResolvedMargins{},
                        isLtr,
                        availableWidth,
                        sizing.widthRequest,
                        fragments,
                        childrenLineBoxes,
                        currentLineBox,
                        currentLineBoxIndex,
                        pendingStart,
                        pendingWidth,
                        lastFragmentHasBreakOpportunity
                    );
                } else if (flow[i].kind == InlineFlowKind::Atomic) {
                    const auto sizeResult = evaluateSize(tree, child, frameInfo, constraints, childRequest, sizeCache);
                    const auto& run = child->textBidiInput->runs.front();
                    appendAtomicInlineFragment(
                        atoms,
                        sizeResult,
                        run,
                        margins,
                        availableWidth,
                        sizing.widthRequest,
                        fragments,
                        childrenLineBoxes,
                        currentLineBox,
                        currentLineBoxIndex,
                        pendingStart,
                        pendingWidth,
                        lastFragmentHasBreakOpportunity
                    );
                } else if (flow[i].kind == InlineFlowKind::Open) {
                    const auto padding = resolvePadding(childRequest);
                    const auto border = resolveBorder(childRequest);
                    const auto* resolvedPaddingStart = std::get_if<float>(isLtr ? &padding.left : &padding.right);
                    const auto* resolvedBorderStart = std::get_if<float>(isLtr ? &border.left : &border.right);
                    const float margin = isLtr ? margins.left : margins.right;
                    float width = 0.0f;

                    if (resolvedPaddingStart) {
                        width += *resolvedPaddingStart;
                    }

                    if (resolvedBorderStart) {
                        width += *resolvedBorderStart;
                    }

                    if (lastFragmentHasBreakOpportunity) {
                        commitPendingFragments(
                            sizing.widthRequest,
                            availableWidth,
                            fragments,
                            childrenLineBoxes,
                            currentLineBox,
                            currentLineBoxIndex,
                            pendingStart,
                            pendingWidth
                        );
                    }

                    const auto fontMetrics = runtime::ContextManager::getContext().glyphCache.metrics(
                        child->inheritedProperties.font,
                        child->inheritedProperties.fontSize.resolveOr(Size::px(0.0f))
                    );
                    fragments.push_back({
                        .kind = InlineFlowKind::Open,
                        .width = width,
                        .height = fontMetrics.lineHeight * child->inheritedProperties.lineHeight,
                        .ascent = fontMetrics.ascent,
                        .descent = fontMetrics.descent,
                        .leadingMargin = margin
                    });
                    pendingWidth += margin + width;
                    lastFragmentHasBreakOpportunity = false;
                } else if (flow[i].kind == InlineFlowKind::Placeholder) {
                    fragments.push_back({.kind = InlineFlowKind::Placeholder, .bidiLevel = layout::InheritedLevel{}});
                } else if (flow[i].kind == InlineFlowKind::Close) {
                    const auto padding = resolvePadding(childRequest);
                    const auto border = resolveBorder(childRequest);
                    const auto* resolvedPaddingEnd = std::get_if<float>(isLtr ? &padding.right : &padding.left);
                    const auto* resolvedBorderEnd = std::get_if<float>(isLtr ? &border.right : &border.left);
                    const float margin = isLtr ? margins.right : margins.left;
                    float width = 0.0f;

                    if (resolvedPaddingEnd) {
                        width += *resolvedPaddingEnd;
                    }

                    if (resolvedBorderEnd) {
                        width += *resolvedBorderEnd;
                    }

                    const auto fontMetrics = runtime::ContextManager::getContext().glyphCache.metrics(
                        child->inheritedProperties.font,
                        child->inheritedProperties.fontSize.resolveOr(Size::px(0.0f))
                    );
                    fragments.push_back({
                        .kind = InlineFlowKind::Close,
                        .width = width,
                        .height = fontMetrics.lineHeight * child->inheritedProperties.lineHeight,
                        .ascent = fontMetrics.ascent,
                        .descent = fontMetrics.descent,
                        .trailingMargin = margin
                    });
                    pendingWidth += margin + width;
                }

                const size_t childFragmentCount = fragments.size() - childFragmentStart;

                for (size_t f = childFragmentStart; f < fragments.size(); ++f) {
                    fragments[f].elementIndex = child->id;
                    fragments[f].elementFragmentIndex = f - childFragmentStart;
                    fragments[f].elementFragmentCount = childFragmentCount;

                    if (flow[i].kind == InlineFlowKind::Atomic) {
                        fragments[f].leadingMargin = isLtr ? margins.left : margins.right;
                        fragments[f].trailingMargin = isLtr ? margins.right : margins.left;
                    }
                }
            } else {
                commitPendingFragments(
                    sizing.widthRequest,
                    availableWidth,
                    fragments,
                    childrenLineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    pendingStart,
                    pendingWidth
                );
                lastFragmentHasBreakOpportunity = false;

                if (currentLineBox.fragmentCount > 0) {
                    childrenLineBoxes.push_back(std::move(currentLineBox));
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }
            }

            auto& nodeRange = nodeFragments.try_emplace(child->id, layout::InlineFragmentRange{.start = childFragmentStart}).first->second;
            nodeRange.count = fragments.size() - nodeRange.start;
        }

        commitPendingFragments(
            sizing.widthRequest,
            availableWidth,
            fragments,
            childrenLineBoxes,
            currentLineBox,
            currentLineBoxIndex,
            pendingStart,
            pendingWidth
        );

        if (currentLineBox.fragmentCount > 0) {
            childrenLineBoxes.push_back(std::move(currentLineBox));
        }

        reorderLineFragments(*context, isLtr);

        if (sizing.trackIntrinsicWidth && sizing.widthRequest != IntrinsicRequest::None) {
            if (sizing.widthRequest != IntrinsicRequest::Minimum) {
                InlineSizingInput minimumSizing {
                    .availableWidth = std::monostate{},
                    .widthRequest = IntrinsicRequest::Minimum,
                    .trackIntrinsicWidth = false,
                };
                auto minContext = buildInlineBoxes(tree, node, frameInfo, constraints, request, minimumSizing, sizeCache);
                context->minFragments = std::move(minContext->fragments);
                context->minLineBoxes = std::move(minContext->lineBoxes);
                context->minNodeFragments = std::move(minContext->nodeFragments);
            }

            if (sizing.widthRequest != IntrinsicRequest::Maximum) {
                InlineSizingInput maximumSizing {
                    .availableWidth = std::monostate{},
                    .widthRequest = IntrinsicRequest::Maximum,
                    .trackIntrinsicWidth = false,
                };
                auto maxContext = buildInlineBoxes(tree, node, frameInfo, constraints, request, maximumSizing, sizeCache);
                context->maxFragments = std::move(maxContext->fragments);
                context->maxLineBoxes = std::move(maxContext->lineBoxes);
                context->maxNodeFragments = std::move(maxContext->nodeFragments);
            }

            const auto& minLineBoxes = context->minLineBoxes.empty() ? context->lineBoxes : context->minLineBoxes;
            const auto& maxLineBoxes = context->maxLineBoxes.empty() ? context->lineBoxes : context->maxLineBoxes;

            if (!minLineBoxes.empty() || !maxLineBoxes.empty()) {
                float minContent = 0.0f;
                for (const auto& lineBox : minLineBoxes) {
                    minContent = std::max(minContent, lineBox.width);
                }
                float maxContent = 0.0f;

                for (const auto& lineBox : maxLineBoxes) {
                    maxContent = std::max(maxContent, lineBox.width);
                }
                context->intrinsicSizes = layout::IntrinsicSizes{.minimum = minContent, .maximum = maxContent};
            }
        }

        return context;
    }
}
