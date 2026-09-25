#include "tree_node.hpp"
#include "fragment_types.hpp"
#include "layout/sizing.hpp"
#include "layout/layout.hpp"
#include "render_tree.hpp"
#include "utf8.hpp"
#include "text/textShaper.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <cstdlib>
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

    Result<std::vector<std::optional<bidi::TextBidiInput>>> prepareChildBidiInputs(
        TreeNode* parent,
        layout::Direction baseDirection
    ) {
        std::vector<std::optional<bidi::TextBidiInput>> inputs(parent->children.size());

        size_t sequenceStart = 0;
        while (sequenceStart < parent->children.size()) {
            while (sequenceStart < parent->children.size() &&
                   !parent->children[sequenceStart]->element->isInline()) {
                ++sequenceStart;
            }
            if (sequenceStart == parent->children.size()) break;

            size_t sequenceEnd = sequenceStart;
            std::string paragraph;
            std::vector<size_t> childStarts;
            std::vector<size_t> childLengths;
            while (sequenceEnd < parent->children.size()) {
                auto* child = parent->children[sequenceEnd].get();
                if (!child->element->isInline()) break;

                childStarts.push_back(paragraph.size());

                if (auto childText = getText(child)) {
                    std::string bidiText = *childText;
                    const auto whiteSpace = getWhiteSpace(child)
                        .value_or(WhiteSpace::Normal);
                    if (whiteSpace == WhiteSpace::Normal || whiteSpace == WhiteSpace::NoWrap) {
                        for (char& byte : bidiText) {
                            if (byte == '\r' || byte == '\n') byte = ' ';
                        }
                    }

                    paragraph += bidiText;
                } else {
                    paragraph += "\xEF\xBF\xBC";
                }

                childLengths.push_back(paragraph.size() - childStarts.back());
                ++sequenceEnd;
            }

            auto resolvedContext = bidi::TextBidiContext::create(
                paragraph,
                baseDirection == layout::Direction::rtl
                    ? bidi::BidiBaseDirection::Rtl
                    : bidi::BidiBaseDirection::Ltr
            );
            if (!resolvedContext) return std::unexpected{resolvedContext.error()};
            auto context = std::make_shared<bidi::TextBidiContext>(std::move(*resolvedContext));

            for (size_t i = sequenceStart; i < sequenceEnd; ++i) {
                const size_t childStart = childStarts[i - sequenceStart];
                const size_t childLength = childLengths[i - sequenceStart];
                const size_t childEnd = childStart + childLength;
                std::vector<bidi::TextShapingRun> childRuns;
                for (const auto& run : context->runs()) {
                    const size_t runEnd = run.byteStart + run.byteLength;
                    const size_t start = std::max(childStart, run.byteStart);
                    const size_t end = std::min(childEnd, runEnd);
                    if (start >= end) continue;
                    childRuns.push_back({
                        .byteStart = start - childStart,
                        .byteLength = end - start,
                        .level = run.level,
                        .scriptTag = run.scriptTag
                    });
                }
                inputs[i] = bidi::TextBidiInput{
                    .context = context,
                    .paragraphByteStart = childStart,
                    .byteLength = childLength,
                    .runs = childRuns
                };
            }
            sequenceStart = sequenceEnd;
        }

        return inputs;
    }

    std::optional<WhiteSpace> getWhiteSpace(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "whiteSpace"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<WhiteSpace>(resp);
        }
        return std::nullopt;
    }

    std::optional<WordBreak> getWordBreak(TreeNode* node) {
        std::any request{DescriptorPayload{GetField{.name = "wordBreak"}}};
        auto resp = node->element->request(RequestTarget::Descriptor, request);
        if (resp.has_value()) {
            return std::any_cast<WordBreak>(resp);
        }
        return std::nullopt;
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
        std::vector<LineFragment>& fragments,
        LineBox& lineBox,
        size_t lineBoxIndex
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

            for (size_t i = runStart; i < runEnd; ++i) {
                const auto& cluster = shapedRun.clusters[i];
                atomStart = std::min(atomStart, cluster.glyphStart);
                atomEnd = std::max(atomEnd, cluster.glyphStart + cluster.glyphCount);
                for (size_t glyph = 0; glyph < cluster.glyphCount; ++glyph) {
                    width += atoms[cluster.glyphStart + glyph].width;
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
                .width = width,
                .atomStart = atomStart,
                .atomCount = atomEnd - atomStart,
                .textByteStart = byteStart,
                .textByteLength = byteEnd - byteStart,
                .bidiLevel = run.bidiLevel,
                .lineBoxIndex = lineBoxIndex
            };

            if (lineBox.fragmentCount == 0) {
                lineBox.fragmentStart = fragments.size();
            }

            lineBox.pushFragment(fragment);
            fragments.push_back(fragment);
        }
    }

    void reorderLineFragments(layout::InlineFormattingContext& context, bool isLtr) {
        std::vector<LineFragment*> fragments;
        fragments.reserve(context.fragments.size());

        std::unordered_map<size_t, size_t> elementFragmentIndices;

        for (size_t lineIndex = 0; lineIndex < context.lineBoxes.size(); ++lineIndex) {
            auto& lineBox = context.lineBoxes[lineIndex];
            fragments.clear();
            int maximumLevel = 0;
            int minimumOddLevel = -1;

            for (size_t i = 0; i < lineBox.fragmentCount; ++i) {
                auto& fragment = context.fragments[lineBox.fragmentStart + i];
                fragments.push_back(&fragment);
                maximumLevel = std::max(maximumLevel, static_cast<int>(fragment.bidiLevel));
                if ((fragment.bidiLevel & 1u) != 0 &&
                    (minimumOddLevel == -1 || fragment.bidiLevel < minimumOddLevel)) {
                    minimumOddLevel = fragment.bidiLevel;
                }
            }

            if (minimumOddLevel != -1) {
                for (int level = maximumLevel; level >= minimumOddLevel; --level) {
                    size_t start = 0;
                    while (start < fragments.size()) {
                        while (start < fragments.size() &&
                               fragments[start]->bidiLevel < level) {
                            ++start;
                        }

                        size_t end = start;
                        while (end < fragments.size() &&
                               fragments[end]->bidiLevel >= level) {
                            ++end;
                        }

                        std::reverse(
                            fragments.begin() + start,
                            fragments.begin() + end
                        );
                        start = end;
                    }
                }
            }

            for (size_t i = 0; i < fragments.size(); ++i) {
                auto* fragment = isLtr ? fragments[i] : fragments[fragments.size() - 1 - i];
                fragment->elementFragmentIndex = elementFragmentIndices[fragment->elementIndex]++;

                if (fragment->elementFragmentIndex != 0) {
                    fragment->leadingMargin = 0.0f;
                    fragment->leadingPadding = 0.0f;
                }

                if (fragment->elementFragmentIndex + 1 != fragment->elementFragmentCount) {
                    fragment->trailingMargin = 0.0f;
                    fragment->trailingPadding = 0.0f;
                }
            }

            float offset = 0.0f;

            for (auto* fragment : fragments) {
                fragment->offset = isLtr ? offset + fragment->leadingMargin + fragment->leadingPadding : offset + fragment->trailingMargin + fragment->trailingPadding;
                offset += fragment->leadingMargin + fragment->leadingPadding + fragment->width + fragment->trailingPadding + fragment->trailingMargin;
            }

            assert(std::abs(offset - lineBox.width) < 0.001f);
        }
    }

    bool shouldTakeSoftBreak(
        std::optional<IntrinsicRequest> widthRequest,
        bool hasBreakOpportunity,
        bool lineHasContent,
        float prospectiveWidth,
        const SizeState& availableWidth
    ) {
        if (!hasBreakOpportunity || !lineHasContent) return false;

        if (widthRequest == IntrinsicRequest::Minimum) return true;
        if (widthRequest == IntrinsicRequest::Maximum) return false;

        const auto* resolvedAvailableWidth = std::get_if<float>(&availableWidth);
        return resolvedAvailableWidth && prospectiveWidth > *resolvedAvailableWidth;
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
        
        float width = margins.left + margins.right + atoms.front().width;

        if (shouldTakeSoftBreak(
                widthRequest,
                true,
                currentLineBox.fragmentCount > 0,
                currentLineBox.width + width,
                availableWidth
            )) {
            lineBoxes.push_back(std::move(currentLineBox));
            currentLineBox = {};
            currentLineBoxIndex++;
        }

        LineFragment fragment {
            .width = atoms.front().width,
            .atomStart = 0,
            .atomCount = atoms.size(),
            .bidiLevel = run.level,
            .lineBoxIndex = currentLineBoxIndex
        };
        if (currentLineBox.fragmentCount == 0) {
            currentLineBox.fragmentStart = fragments.size();
        }

        currentLineBox.pushFragment(fragment);
        currentLineBox.width += margins.left + margins.right;
        fragments.push_back(fragment);
        lastFragmentHasBreakOpportunity = true;
    }

    void appendTextLineFragments(
        const std::string& text,
        const ShapedRun& shapedRun,
        const std::vector<Atom>& atoms,
        WhiteSpace whiteSpace,
        WordBreak wordBreak,
        ResolvedMargins margins,
        const PaddingResult& padding,
        bool isLtr,
        const SizeState& availableWidth,
        std::optional<IntrinsicRequest> widthRequest,
        std::vector<LineFragment>& fragments,
        std::vector<LineBox>& lineBoxes,
        LineBox& currentLineBox,
        size_t& currentLineBoxIndex,
        bool& lastFragmentHasBreakOpportunity
    ) {
        const float marginStart = isLtr ? margins.left : margins.right;
        const float marginEnd = isLtr ? margins.right : margins.left;
        const auto* paddingStart = std::get_if<float>(isLtr ? &padding.left : &padding.right);
        const auto* paddingEnd = std::get_if<float>(isLtr ? &padding.right : &padding.left);

        const bool preserveLineFeeds = whiteSpace == WhiteSpace::Pre ||  whiteSpace == WhiteSpace::PreWrap;
        const bool allowSoftWrap = whiteSpace == WhiteSpace::Normal || whiteSpace == WhiteSpace::PreWrap;
        const bool breakInsideWords = allowSoftWrap && wordBreak == WordBreak::BreakAll;

        float runningWidth = marginStart;
        float runningEdge = marginStart;

        if (paddingStart) {
            runningWidth += *paddingStart;
            runningEdge += *paddingStart;
        }
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

                    if (paddingEnd) {
                        runningWidth += *paddingEnd;
                        runningEdge += *paddingEnd;
                    }
                }

                runningAtomCount += cluster.glyphCount;

                if (allowSoftWrap && shouldTakeSoftBreak(
                        widthRequest,
                        lastFragmentHasBreakOpportunity,
                        currentLineBox.fragmentCount > 0,
                        currentLineBox.width + runningWidth,
                        availableWidth
                    )) {
                    lineBoxes.push_back(std::move(currentLineBox));
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                pushRunFragments(
                    shapedRun,
                    atoms,
                    runningClusterStart,
                    idx + 1,
                    runningWidth - runningEdge,
                    fragments,
                    currentLineBox,
                    currentLineBoxIndex
                );
                currentLineBox.width += runningEdge;

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
                float prospectiveWidth = currentLineBox.width + runningWidth + width;

                if (shouldTakeSoftBreak(
                        widthRequest,
                        true,
                        currentLineBox.fragmentCount > 0 || runningAtomCount > 0,
                        prospectiveWidth,
                        availableWidth
                    )) {
                    bool hadPendingAtoms = runningAtomCount > 0;
                    if (runningAtomCount > 0) {
                        pushRunFragments(
                            shapedRun,
                            atoms,
                            runningClusterStart,
                            idx,
                            runningWidth - runningEdge,
                            fragments,
                            currentLineBox,
                            currentLineBoxIndex
                        );
                        currentLineBox.width += runningEdge;
                    }

                    lineBoxes.push_back(std::move(currentLineBox));
                    currentLineBox = {};
                    currentLineBoxIndex++;
                    lastFragmentHasBreakOpportunity = false;

                    if (hadPendingAtoms) {
                        runningWidth = 0.0f;
                        runningEdge = 0.0f;
                    }
                    runningAtomCount = 0;
                    runningClusterStart = idx;
                }

                runningWidth += width;
                runningAtomCount += cluster.glyphCount;
                idx++;
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

                if (paddingEnd) {
                    runningWidth += *paddingEnd;
                    runningEdge += *paddingEnd;
                }
            }

            if (allowSoftWrap && shouldTakeSoftBreak(
                    widthRequest,
                    lastFragmentHasBreakOpportunity,
                    currentLineBox.fragmentCount > 0,
                    currentLineBox.width + runningWidth,
                    availableWidth
                )) {
                lineBoxes.push_back(std::move(currentLineBox));
                currentLineBox = {};
                currentLineBoxIndex++;
            }

            pushRunFragments(
                shapedRun,
                atoms,
                runningClusterStart,
                idx,
                runningWidth - runningEdge,
                fragments,
                currentLineBox,
                currentLineBoxIndex
            );
            currentLineBox.width += runningEdge;
            lastFragmentHasBreakOpportunity = true;

            runningWidth = 0.0;
            runningEdge = 0.0;
            runningAtomCount = 0;
            runningClusterStart = idx;
        }

        if (runningAtomCount > 0) {
            runningWidth += marginEnd;
            runningEdge += marginEnd;

            if (paddingEnd) {
                runningWidth += *paddingEnd;
                runningEdge += *paddingEnd;
            }

            if (allowSoftWrap && shouldTakeSoftBreak(
                    widthRequest,
                    lastFragmentHasBreakOpportunity,
                    currentLineBox.fragmentCount > 0,
                    currentLineBox.width + runningWidth,
                    availableWidth
                )) {
                lineBoxes.push_back(std::move(currentLineBox));
                currentLineBox = {};
                currentLineBoxIndex++;
            }

            pushRunFragments(
                shapedRun,
                atoms,
                runningClusterStart,
                shapedRun.clusters.size(),
                runningWidth - runningEdge,
                fragments,
                currentLineBox,
                currentLineBoxIndex
            );
            currentLineBox.width += runningEdge;
            lastFragmentHasBreakOpportunity = false;
        }
    }
    

    layout::InlineFormattingInput buildIsolatedInlineBoxes(
        RenderTree& tree, TreeNode* node, const FrameInfo& frameInfo,
        const Constraints& constraints, const SizeRequest& request, const InlineSizingInput& sizing,
        std::optional<std::unordered_map<size_t, SizeResult>&> sizeCache) {
        auto context = std::make_shared<layout::InlineFormattingContext>();
        auto& fragments = context->fragments;
        auto& lineBoxes = context->lineBoxes;
        LineBox currentLineBox{};
        size_t currentLineBoxIndex = 0;
        bool lastFragmentHasBreakOpportunity = false;
        SizeState availableWidth = calculateSize(sizing.availableWidth, std::monostate{});
        const bool isLtr = constraints.inheritedProperties.direction == layout::Direction::ltr;

        if (node->element->isInline()) {
            auto textResp = getText(node);
            ResolvedMargins margins {};
            auto& atoms = node->atomized->atoms;
            PaddingResult padding {};

            if (textResp.has_value()) {
                padding = resolvePadding(request);

                appendTextLineFragments(
                    *textResp,
                    *getShapedRun(node),
                    atoms,
                    getWhiteSpace(node).value_or(WhiteSpace::Normal),
                    getWordBreak(node).value_or(WordBreak::Normal),
                    margins,
                    padding,
                    isLtr,
                    availableWidth,
                    sizing.widthRequest,
                    fragments,
                    lineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    lastFragmentHasBreakOpportunity
                );
            } else {
                const auto sizeResult = evaluateSize(tree, node, frameInfo, constraints, request, sizeCache);
                const auto& run = node->textBidiInput->runs.front();
                appendAtomicInlineFragment(
                    atoms,
                    sizeResult,
                    run,
                    margins,
                    availableWidth,
                    sizing.widthRequest,
                    fragments,
                    lineBoxes,
                    currentLineBox,
                    currentLineBoxIndex,
                    lastFragmentHasBreakOpportunity
                );
            }

            const auto* paddingStart = std::get_if<float>(isLtr ? &padding.left : &padding.right);
            const auto* paddingEnd = std::get_if<float>(isLtr ? &padding.right : &padding.left);

            for (size_t f = 0; f < fragments.size(); ++f) {
                fragments[f].elementIndex = 0;
                fragments[f].elementFragmentIndex = f;
                fragments[f].elementFragmentCount = fragments.size();
                fragments[f].leadingMargin = isLtr ? margins.left : margins.right;
                fragments[f].trailingMargin = isLtr ? margins.right : margins.left;

                if (paddingStart) {
                    fragments[f].leadingPadding = *paddingStart;
                }

                if (paddingEnd) {
                    fragments[f].trailingPadding = *paddingEnd;
                }
            }
        }

        if (currentLineBox.fragmentCount > 0)
            lineBoxes.push_back(std::move(currentLineBox));

        reorderLineFragments(*context, isLtr);

        if (sizing.trackIntrinsicWidth && sizing.widthRequest != IntrinsicRequest::None) {
            if (sizing.widthRequest != IntrinsicRequest::Minimum) {
                InlineSizingInput minimumSizing {
                    .availableWidth = std::monostate{},
                    .widthRequest = IntrinsicRequest::Minimum,
                    .trackIntrinsicWidth = false,
                };
                auto minContext = buildIsolatedInlineBoxes(tree, node, frameInfo, constraints, request, minimumSizing, sizeCache).context;
                context->minFragments = std::move(minContext->fragments);
                context->minLineBoxes = std::move(minContext->lineBoxes);
            }

            if (sizing.widthRequest != IntrinsicRequest::Maximum) {
                InlineSizingInput maximumSizing {
                    .availableWidth = std::monostate{},
                    .widthRequest = IntrinsicRequest::Maximum,
                    .trackIntrinsicWidth = false,
                };
                auto maxContext = buildIsolatedInlineBoxes(tree, node, frameInfo, constraints, request, maximumSizing, sizeCache).context;
                context->maxFragments = std::move(maxContext->fragments);
                context->maxLineBoxes = std::move(maxContext->lineBoxes);
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

        return {
            .context = context,
            .fragments = {.start = 0, .count = fragments.size()},
            .minFragments = {.start = 0, .count = context->minFragments.size()},
            .maxFragments = {.start = 0, .count = context->maxFragments.size()},
        };
    }

    std::shared_ptr<layout::InlineFormattingContext> buildInlineBoxes(
        RenderTree& tree, TreeNode* node, const FrameInfo& frameInfo,
        const Constraints& constraints, const SizeRequest& request, const InlineSizingInput& sizing,
        std::optional<std::unordered_map<size_t, SizeResult>&> sizeCache) {
        bool prevInline = false;
        auto context = std::make_shared<layout::InlineFormattingContext>();
        auto& childrenLineBoxes = context->lineBoxes;
        auto& fragments = context->fragments;
        auto& childFragments = context->childFragments;
        LineBox currentLineBox {};
        size_t currentLineBoxIndex = 0;
        bool lastFragmentHasBreakOpportunity = false;
        SizeState availableWidth = calculateSize(sizing.availableWidth, std::monostate{});
        const bool isLtr = constraints.inheritedProperties.direction == layout::Direction::ltr;


        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto& child = node->children[i];

            size_t childFragmentStart = fragments.size();

            if (child->element->isInline()) {
                auto textResp = getText(child.get());
                auto margins = child->preLayout->resolvedMargins;
                auto& atoms = child->atomized->atoms;

                if (i > 0 && !prevInline && currentLineBox.fragmentCount > 0) {
                    childrenLineBoxes.push_back(std::move(currentLineBox));
                    currentLineBox = {};
                    currentLineBoxIndex++;
                }

                SizeRequest childRequest {
                    .position = child->shared.position,
                    .specified = {.width = child->shared.width, .height = child->shared.height},
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
                PaddingResult padding {};

                if (textResp.has_value()) {
                    padding = resolvePadding(childRequest);

                    appendTextLineFragments(
                        *textResp,
                        *getShapedRun(child.get()),
                        atoms,
                        getWhiteSpace(child.get()).value_or(WhiteSpace::Normal),
                        getWordBreak(child.get()).value_or(WordBreak::Normal),
                        margins,
                        padding,
                        isLtr,
                        availableWidth,
                        sizing.widthRequest,
                        fragments,
                        childrenLineBoxes,
                        currentLineBox,
                        currentLineBoxIndex,
                        lastFragmentHasBreakOpportunity
                    );
                } else {
                    const auto sizeResult = evaluateSize(tree, child.get(), frameInfo, constraints, childRequest, sizeCache);
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
                        lastFragmentHasBreakOpportunity
                    );
                }

                const size_t childFragmentCount = fragments.size() - childFragmentStart;
                const auto* paddingStart = std::get_if<float>(isLtr ? &padding.left : &padding.right);
                const auto* paddingEnd = std::get_if<float>(isLtr ? &padding.right : &padding.left);

                for (size_t f = childFragmentStart; f < fragments.size(); ++f) {
                    fragments[f].elementIndex = i;
                    fragments[f].elementFragmentIndex = f - childFragmentStart;
                    fragments[f].elementFragmentCount = childFragmentCount;
                    fragments[f].leadingMargin = isLtr ? margins.left : margins.right;
                    fragments[f].trailingMargin = isLtr ? margins.right : margins.left;

                    if (paddingStart) {
                        fragments[f].leadingPadding = *paddingStart;
                    }

                    if (paddingEnd) {
                        fragments[f].trailingPadding = *paddingEnd;
                    }
                }

                prevInline = true;
            }else {
                prevInline = false;
            }

            childFragments.push_back({
                .start = childFragmentStart,
                .count = fragments.size() - childFragmentStart
            });
        }

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
                context->minChildFragments = std::move(minContext->childFragments);
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
                context->maxChildFragments = std::move(maxContext->childFragments);
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
