//
//  inline.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "layout/layout.hpp"
#include "fragment_types.hpp"
#include "overloaded.hpp"
#include "layout/sizing.hpp"
#include "render_tree.hpp"
#include "tree_node.hpp"
#include <algorithm>
#include <optional>
#include <simd/vector_types.h>


namespace layout {
    InlineState LayoutEngine::layoutInlineNormalFlow(
        Constraints& constraints,
        simd_float2 currentCursor,
        LayoutInput& layoutInput,
        Atomized& atomized,
        const SizeResult& sizeResult
    ) {
        InlineState lr;
        lr.outOfFlow = false;

        ResolvedMargins margins = constraints.resolvedMargins;

        std::vector<simd_float2> atomOffsets(atomized.atoms.size());

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .sizeResult = sizeResult,
            .margins = margins
        };

        simd_float2 newCursor = resolvePosition(pctx);

        // lr.childConstraints.origin = {0.0f, 0.0f};
        float totalHeight = 0;
        float totalWidth = 0;
        float currentTotalWidth = 0;

        bool isLtr = constraints.inheritedProperties.direction == Direction::ltr;
        bool placedByParent = constraints.parentDisplay == Display::Flex || constraints.parentDisplay == Display::Grid;
        size_t prevLineBoxIndex = -1;

        /*
            there is a few conflicts
            1) this returns the line box derived size
            2) inline boxes get initialized with the actual min and max possible line box width
            3) line box derived size can disagree with the actual min and max possible line box width
            4) that isn't a problem; its completely expected
            5) but it screws up intrinsic measurement
            6) one idea: get rid of the *disagreement*
            7) but the problem is, then cyclical dependencies
            8) but I cant think of a good reason as to why text should even contribute a size... huh? Why does it matter per element
            9) lol what was the point of this collect 

        */

        /*
            unfortunately: inline still needs to collect *this sizing info*
            this breaks the necessary split I wanted
            but kind of derives an interesting split
            that carries with flex
            direct content sizing -> handled by layout
            finalizing the actual size of an object -> handled by evaluate size
        */
        auto lineFragments = constraints.inlineFormatting.lineFragments();
        auto lineBoxes = constraints.inlineFormatting.lineBoxes();

        if (constraints.edgeIntent.edgeDisplayMode == Display::Inline && !lineFragments.empty() && constraints.inlineFormatting.fragments.start > 0) {
            const auto& previousFragment = constraints.inlineFormatting.context->fragments[constraints.inlineFormatting.fragments.start - 1];
            prevLineBoxIndex = previousFragment.lineBoxIndex;
        }

        lr.prevInlineHeight = constraints.prevInlineHeight;

        lr.childConstraints = constraints;
        lr.childConstraints.edgeIntent = {
            .edgeDisplayMode = Display::Inline,
            .collapsable = false,
        };

        size_t fragmentIdx = 0;
        for (auto it = lineFragments.begin(); it != lineFragments.end(); ++it, ++fragmentIdx) {
            const LineFragment& fragment = *it;

            const auto& lineBox = lineBoxes[fragment.lineBoxIndex];
            float offset = fragment.offset;
            float lineOffset = 0.0f;
            bool definiteAvailableWidth = std::holds_alternative<float>(constraints.availableWidth);
            float availableWidth = definiteAvailableWidth
                ? std::get<float>(constraints.availableWidth)
                : 0.0f;
            switch (constraints.inheritedProperties.textAlign) {
                case TextAlign::Start:
                    lineOffset = isLtr || !definiteAvailableWidth ? 0.0f : availableWidth - lineBox.width;
                    break;
                case TextAlign::Left:
                    break;
                case TextAlign::Center:
                    lineOffset = !definiteAvailableWidth ? 0.0f : (availableWidth - lineBox.width) / 2.0f;
                    break;
                case TextAlign::Right:
                    lineOffset = !definiteAvailableWidth ? 0.0f : availableWidth - lineBox.width;
                    break;
            }
            float startingX = constraints.origin.x + lineOffset + offset;

            newCursor.x = startingX;

            if (fragmentIdx == 0 && !placedByParent && constraints.edgeIntent.edgeDisplayMode != Display::Inline) {
                if (constraints.edgeIntent.collapsable) {
                    newCursor.y += std::max(margins.top, constraints.edgeIntent.intent);
                } else {
                    newCursor.y += margins.top + constraints.edgeIntent.intent;
                }
            }

            // did we move to a new line
                // true if indices dont match and the line box index is -1
                // including on first line
            if (fragment.lineBoxIndex != prevLineBoxIndex && prevLineBoxIndex != -1) 
            {
                newCursor.y += lineBoxes[prevLineBoxIndex].height;
                newCursor.x = startingX;
                totalWidth = std::max(currentTotalWidth, totalWidth);
                currentTotalWidth = 0;
            }

            // do we need a new box fragment
                // true on every line and true when we're the starting fragments
            if (it == lineFragments.begin() || fragment.lineBoxIndex != prevLineBoxIndex) {
                totalHeight += lineBox.height;

                BoxFragment boxFragment;
                boxFragment.computedBox.x = startingX;
                boxFragment.brokenLeft = true;
                boxFragment.brokenRight = true;
                boxFragment.computedInnerBox.y = newCursor.y + lineBox.ascent - lineFragments.front().ascent;
                boxFragment.computedInnerBox.height = lineFragments.front().ascent + lineFragments.front().descent;
                lr.boxFragments.push_back(boxFragment);
            }

            switch (fragment.kind) {
                case InlineFlowKind::Atomic: {
                    lr.boxFragments.back().brokenLeft = false;
                    lr.boxFragments.back().brokenRight = false;
                    break;
                }
                case InlineFlowKind::Open: {
                    if (it == lineFragments.begin()) {
                        if (isLtr) {
                            lr.boxFragments.back().brokenLeft = false;
                        } else {
                            lr.boxFragments.back().brokenRight = false;
                        }
                    }
                    lr.childConstraints.cursor = newCursor;
                    break;
                }
                case InlineFlowKind::Close: {
                    if (std::next(it) == lineFragments.end()) {
                        if (isLtr) {
                            lr.boxFragments.back().brokenRight = false;
                        } else {
                            lr.boxFragments.back().brokenLeft = false;
                        }
                    }
                    break;
                }
                default: {
                    break;
                }
            }

            auto& lineBoxFragment = lr.boxFragments.back().computedBox;
            const float lineBoxFragmentEnd = std::max(lineBoxFragment.x + lineBoxFragment.width, startingX + fragment.width);
            lineBoxFragment.x = std::min(lineBoxFragment.x, startingX);
            lineBoxFragment.width = lineBoxFragmentEnd - lineBoxFragment.x;

            size_t atomIndex = fragment.atomStart;
            for (size_t i = 0; i < fragment.atomCount && atomIndex < atomized.atoms.size(); ++i, ++atomIndex) {
                auto& atom = atomized.atoms[atomIndex];

                atomOffsets[atomIndex] = newCursor + simd_float2{0.0f, lineBox.ascent - atom.ascent};
                newCursor.x += atom.width;
            }

            currentTotalWidth += fragment.width;

            lr.prevInlineHeight = lineBox.height;
            prevLineBoxIndex = fragment.lineBoxIndex;
        }

        totalWidth = std::max(currentTotalWidth, totalWidth);

        const auto* resolvedPaddingTop = std::get_if<float>(&sizeResult.padding.top);
        const auto* resolvedPaddingRight = std::get_if<float>(&sizeResult.padding.right);
        const auto* resolvedPaddingBottom = std::get_if<float>(&sizeResult.padding.bottom);
        const auto* resolvedPaddingLeft = std::get_if<float>(&sizeResult.padding.left);
        const auto* resolvedBorderTop = std::get_if<float>(&sizeResult.border.top);
        const auto* resolvedBorderRight = std::get_if<float>(&sizeResult.border.right);
        const auto* resolvedBorderBottom = std::get_if<float>(&sizeResult.border.bottom);
        const auto* resolvedBorderLeft = std::get_if<float>(&sizeResult.border.left);

        for (auto& boxFragment : lr.boxFragments) {
            boxFragment.computedPaddingBox.x = boxFragment.computedBox.x;
            boxFragment.computedPaddingBox.width = boxFragment.computedBox.width;
            boxFragment.computedPaddingBox.y = boxFragment.computedInnerBox.y;
            boxFragment.computedPaddingBox.height = boxFragment.computedInnerBox.height;

            if (!boxFragment.brokenLeft && resolvedBorderLeft) {
                boxFragment.computedPaddingBox.x += *resolvedBorderLeft;
                boxFragment.computedPaddingBox.width -= *resolvedBorderLeft;
            }

            if (!boxFragment.brokenRight && resolvedBorderRight) {
                boxFragment.computedPaddingBox.width -= *resolvedBorderRight;
            }

            if (resolvedPaddingTop) {
                boxFragment.computedPaddingBox.y -= *resolvedPaddingTop;
                boxFragment.computedPaddingBox.height += *resolvedPaddingTop;
            }

            if (resolvedPaddingBottom) {
                boxFragment.computedPaddingBox.height += *resolvedPaddingBottom;
            }

            boxFragment.computedInnerBox.x = boxFragment.computedPaddingBox.x;
            boxFragment.computedInnerBox.width = boxFragment.computedPaddingBox.width;

            if (!boxFragment.brokenLeft && resolvedPaddingLeft) {
                boxFragment.computedInnerBox.x += *resolvedPaddingLeft;
                boxFragment.computedInnerBox.width -= *resolvedPaddingLeft;
            }

            if (!boxFragment.brokenRight && resolvedPaddingRight) {
                boxFragment.computedInnerBox.width -= *resolvedPaddingRight;
            }

            boxFragment.computedBox.y = boxFragment.computedPaddingBox.y;
            boxFragment.computedBox.height = boxFragment.computedPaddingBox.height;

            if (resolvedBorderTop) {
                boxFragment.computedBox.y -= *resolvedBorderTop;
                boxFragment.computedBox.height += *resolvedBorderTop;
            }

            if (resolvedBorderBottom) {
                boxFragment.computedBox.height += *resolvedBorderBottom;
            }
        }

        float minWidth = totalWidth;
        auto minLineFragments = constraints.inlineFormatting.minLineFragments();
        if (!minLineFragments.empty()) {
            minWidth = 0.0f;
            float currentWidth = 0.0f;
            size_t prevIndex = -1;
            for (const LineFragment& fragment : minLineFragments) {
                if (fragment.lineBoxIndex != prevIndex && prevIndex != -1) {
                    minWidth = std::max(currentWidth, minWidth);
                    currentWidth = 0.0f;
                }

                currentWidth += fragment.width;

                prevIndex = fragment.lineBoxIndex;
            }
            minWidth = std::max(currentWidth, minWidth);
        }

        float maxWidth = totalWidth;
        auto maxLineFragments = constraints.inlineFormatting.maxLineFragments();
        if (!maxLineFragments.empty()) {
            maxWidth = 0.0f;
            float currentWidth = 0.0f;
            size_t prevIndex = -1;
            for (const LineFragment& fragment : maxLineFragments) {
                if (fragment.lineBoxIndex != prevIndex && prevIndex != -1) {
                    maxWidth = std::max(currentWidth, maxWidth);
                    currentWidth = 0.0f;
                }

                currentWidth += fragment.width;

                prevIndex = fragment.lineBoxIndex;
            }
            maxWidth = std::max(currentWidth, maxWidth);
        }

        lr.widthIntrinsicSizes = IntrinsicSizes {.minimum = minWidth, .maximum = maxWidth};
        lr.heightIntrinsicSizes = IntrinsicSizes {.minimum = totalHeight, .maximum = totalHeight};

        // padding / nor child available space is really relevant or correct for inline contaienrs

        // lr.childConstraints.cursor = {0, 0};
        // lr.childConstraints.availableWidth = Size::px(totalWidth - paddingLeft - paddingRight);
        // lr.childConstraints.availableHeight = Size::px(totalHeight - paddingTop - paddingBottom);
        // lr.childConstraints.frameInfo = constraints.frameInfo;

        lr.atomOffsets = atomOffsets;

        lr.siblingCursor = newCursor;

        lr.edgeIntent = {
            .edgeDisplayMode = Display::Inline,
            .collapsable = false,
        };

        return lr;
    }

    InlineState LayoutEngine::layoutInlineOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        InlineState lr;
        lr.outOfFlow = true;

        ResolvedMargins margins = constraints.resolvedMargins;

        std::vector<simd_float2> atomOffsets(atomized.atoms.size());

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .sizeResult = sizeResult,
            .margins = margins
        };

        simd_float2 newCursor = resolvePosition(pctx);
        float originX = newCursor.x;

        // lr.childConstraints.origin = {0.0f, 0.0f};

        float lineHeight = 0;
        float totalHeight = 0;
        float totalWidth = 0;
        float currentTotalWidth = 0;
        float minX = newCursor.x;
        float maxX = newCursor.x;
        float minContentX = newCursor.x;
        float maxContentX = newCursor.x;
        float minY = newCursor.y;

        bool isLtr = constraints.inheritedProperties.direction == Direction::ltr;
        size_t prevLineBoxIndex = -1;

        auto lineFragments = constraints.inlineFormatting.lineFragments();
        auto lineBoxes = constraints.inlineFormatting.lineBoxes();
        for (auto it = lineFragments.begin(); it != lineFragments.end(); ++it) {
            const LineFragment& fragment = *it;

            const auto& lineBox = lineBoxes[fragment.lineBoxIndex];
            float offset = fragment.offset;
            float lineOffset = 0.0f;
            bool definiteAvailableWidth = std::holds_alternative<float>(constraints.availableWidth);
            float availableWidth = definiteAvailableWidth
                ? std::get<float>(constraints.availableWidth)
                : 0.0f;
            switch (constraints.inheritedProperties.textAlign) {
                case TextAlign::Start:
                    lineOffset = isLtr || !definiteAvailableWidth ? 0.0f : availableWidth - lineBox.width;
                    break;
                case TextAlign::Left:
                    break;
                case TextAlign::Center:
                    lineOffset = !definiteAvailableWidth ? 0.0f : (availableWidth - lineBox.width) / 2.0f;
                    break;
                case TextAlign::Right:
                    lineOffset = !definiteAvailableWidth ? 0.0f : availableWidth - lineBox.width;
                    break;
            }
            float startingX = originX + lineOffset + offset;

            newCursor.x = startingX;

            if (it == lineFragments.begin()) {
                minX = startingX;
                maxX = startingX;
                minContentX = startingX;
                maxContentX = startingX;
            }

            minContentX = std::min(minContentX, startingX);
            maxContentX = std::max(maxContentX, startingX + fragment.width);

            minX = std::min(minX, startingX);
            maxX = std::max(maxX, startingX + fragment.width);

            if (fragment.lineBoxIndex != prevLineBoxIndex &&
                prevLineBoxIndex != -1
            ) {
                newCursor.y += lineHeight;
                totalHeight += lineHeight;
                lineHeight = 0;
                newCursor.x = startingX;
                totalWidth = std::max(currentTotalWidth, totalWidth);
                currentTotalWidth = 0;
            }


            size_t atomIndex = fragment.atomStart;
            for (size_t i = 0; i < fragment.atomCount && atomIndex < atomized.atoms.size(); ++i, ++atomIndex) {
                auto& atom = atomized.atoms[atomIndex];

                atomOffsets[atomIndex] = newCursor + simd_float2{0.0f, lineBox.ascent - atom.ascent};
                newCursor.x += atom.width;
            }
            lineHeight = lineBox.height;

            currentTotalWidth += fragment.width;

            prevLineBoxIndex = fragment.lineBoxIndex;
        }

        totalHeight += lineHeight;
        totalWidth = std::max(currentTotalWidth, totalWidth);

        const auto* resolvedPaddingTop = std::get_if<float>(&sizeResult.padding.top);
        const auto* resolvedPaddingBottom = std::get_if<float>(&sizeResult.padding.bottom);

        BoxFragment fragment;
        fragment.computedBox = {
            minX,
            minY,
            maxX - minX,
            totalHeight
        };

        if (resolvedPaddingTop) {
            fragment.computedBox.y -= *resolvedPaddingTop;
            fragment.computedBox.height += *resolvedPaddingTop;
        }

        if (resolvedPaddingBottom) {
            fragment.computedBox.height += *resolvedPaddingBottom;
        }

        fragment.computedPaddingBox = fragment.computedBox;

        fragment.computedInnerBox = {
            minContentX,
            minY,
            maxContentX - minContentX,
            totalHeight
        };

        lr.boxFragments = {fragment};

        lr.widthIntrinsicSizes = IntrinsicSizes {.minimum = totalWidth, .maximum = totalWidth};
        lr.heightIntrinsicSizes = IntrinsicSizes {.minimum = totalHeight, .maximum = totalHeight};

        // i dont think any of these are really relevant for inline containers
        // children get lifted up; so these dont need to be set whatsoever

        // lr.childConstraints.cursor = {0, 0};
        // dont set available widths?
        // lr.childConstraints.availableWidth = Size::px(totalWidth - paddingLeft - paddingRight);
        // lr.childConstraints.availableHeight = Size::px(totalHeight - paddingTop - paddingBottom);
        // lr.childConstraints.frameInfo = constraints.frameInfo;

        lr.atomOffsets = atomOffsets;

        lr.siblingCursor = currentCursor;

        return lr;
    }

    std::optional<IntrinsicSizes> inlinePass(
        tree::RenderTree& tree, tree::TreeNode* node, const FrameInfo& frameInfo, InlineState& state,
        Constraints childConstraints, const SizeRequest& sizeRequest,
        bool mutate, std::unordered_map<size_t, SizeResult>& sizeCache
    ) {
        std::optional<IntrinsicSizes> intrinsicResult;

        auto inlineFormatting = childConstraints.inlineFormatting.context;

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto child = node->children[i].get();

            childConstraints.inlineFormatting = {
                .context = inlineFormatting,
                .fragments = inlineFormatting->nodeFragments.at(child->id),
                .minFragments = inlineFormatting->minNodeFragments.empty()
                    ? InlineFragmentRange{}
                    : inlineFormatting->minNodeFragments.at(child->id),
                .maxFragments = inlineFormatting->maxNodeFragments.empty()
                    ? InlineFragmentRange{}
                    : inlineFormatting->maxNodeFragments.at(child->id),
            };

            auto childOutput = tree.layoutRecursive(child, frameInfo, childConstraints, mutate, std::nullopt, sizeRequest.intrinsicWidthRequest, sizeRequest.intrinsicHeightRequest);

            std::visit([&](const auto& childLayout) {
            if (!childLayout.outOfFlow) {
                childConstraints.cursor = childLayout.siblingCursor;
                childConstraints.edgeIntent = childLayout.edgeIntent;
                childConstraints.prevInlineHeight = childLayout.prevInlineHeight;

                state.siblingCursor = childLayout.siblingCursor;
                state.edgeIntent = childLayout.edgeIntent;
                state.prevInlineHeight = childLayout.prevInlineHeight;
            }
            }, childOutput.layout);
        }

        if (sizeRequest.resolvingIntrinsicWidth || sizeRequest.resolvingIntrinsicHeight) {
            intrinsicResult = IntrinsicSizes{};
        }

        if (state.widthIntrinsicSizes) {
            if (sizeRequest.resolvingIntrinsicWidth) {
                intrinsicResult = *state.widthIntrinsicSizes;
            }
        }

        if (state.heightIntrinsicSizes) {
            if (sizeRequest.resolvingIntrinsicHeight) {
                intrinsicResult = *state.heightIntrinsicSizes;
            }
        }

        return intrinsicResult;
    }

}
