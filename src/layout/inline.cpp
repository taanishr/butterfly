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
#include <algorithm>
#include <optional>
#include <simd/vector_types.h>


namespace layout {
    void LineBox::pushFragment(LineFragment& fragment) {
        fragment.offset = currentFragmentOffset;
        currentFragmentOffset += fragment.width;
        width += fragment.width;
        fragmentCount += 1;
    }

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
            lineHeight = constraints.prevInlineHeight;
        }

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

            if (it == lineFragments.begin()) {
                minX = startingX;
                maxX = startingX;
                minContentX = startingX;
                maxContentX = startingX;
            }

            minContentX = std::min(minContentX, startingX);
            maxContentX = std::max(maxContentX, startingX + fragment.width);

            if (isLtr) {
                minX = std::min(minX, startingX - fragment.leadingPadding);
                maxX = std::max(maxX, startingX + fragment.width + fragment.trailingPadding);
            } else {
                minX = std::min(minX, startingX - fragment.trailingPadding);
                maxX = std::max(maxX, startingX + fragment.width + fragment.leadingPadding);
            }

            if (fragmentIdx == 0 && constraints.edgeIntent.edgeDisplayMode != Display::Inline) {
                if (constraints.edgeIntent.collapsable) {
                    newCursor.y += std::max(margins.top, constraints.edgeIntent.intent);
                } else {
                    newCursor.y += margins.top + constraints.edgeIntent.intent;
                }
            }

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
                float usedLineHeight = atom.lineHeight > 0.0f ? atom.lineHeight : atom.height;

                atomOffsets[atomIndex] = newCursor + simd_float2{0.0f, (usedLineHeight - atom.height) / 2.0f};
                newCursor.x += atom.width;
                lineHeight = std::max(lineHeight, usedLineHeight);
            }

            currentTotalWidth += fragment.leadingPadding + fragment.width + fragment.trailingPadding;

            prevLineBoxIndex = fragment.lineBoxIndex;
        }

        totalHeight += lineHeight;
        totalWidth = std::max(currentTotalWidth, totalWidth);

        const auto* resolvedPaddingTop = std::get_if<float>(&sizeResult.padding.top);
        const auto* resolvedPaddingBottom = std::get_if<float>(&sizeResult.padding.bottom);

        lr.computedBox = {
            minX,
            minY,
            maxX - minX,
            totalHeight
        };

        if (resolvedPaddingTop) {
            lr.computedBox.y -= *resolvedPaddingTop;
            lr.computedBox.height += *resolvedPaddingTop;
        }

        if (resolvedPaddingBottom) {
            lr.computedBox.height += *resolvedPaddingBottom;
        }

        lr.computedPaddingBox = lr.computedBox;

        lr.computedInnerBox = {
            minContentX,
            minY,
            maxContentX - minContentX,
            totalHeight
        };

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

                currentWidth += fragment.leadingPadding + fragment.width + fragment.trailingPadding;

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

                currentWidth += fragment.leadingPadding + fragment.width + fragment.trailingPadding;

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
        lr.prevInlineHeight = lineHeight;

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

            if (isLtr) {
                minX = std::min(minX, startingX - fragment.leadingPadding);
                maxX = std::max(maxX, startingX + fragment.width + fragment.trailingPadding);
            } else {
                minX = std::min(minX, startingX - fragment.trailingPadding);
                maxX = std::max(maxX, startingX + fragment.width + fragment.leadingPadding);
            }

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
                float usedLineHeight = atom.lineHeight > 0.0f
                    ? atom.lineHeight
                    : atom.height;

                atomOffsets[atomIndex] = newCursor + simd_float2{
                    0.0f,
                    (usedLineHeight - atom.height) / 2.0f
                };
                newCursor.x += atom.width;
                lineHeight = std::max(lineHeight, usedLineHeight);
            }

            currentTotalWidth += fragment.leadingPadding + fragment.width + fragment.trailingPadding;

            prevLineBoxIndex = fragment.lineBoxIndex;
        }

        totalHeight += lineHeight;
        totalWidth = std::max(currentTotalWidth, totalWidth);

        const auto* resolvedPaddingTop = std::get_if<float>(&sizeResult.padding.top);
        const auto* resolvedPaddingBottom = std::get_if<float>(&sizeResult.padding.bottom);

        lr.computedBox = {
            minX,
            minY,
            maxX - minX,
            totalHeight
        };

        if (resolvedPaddingTop) {
            lr.computedBox.y -= *resolvedPaddingTop;
            lr.computedBox.height += *resolvedPaddingTop;
        }

        if (resolvedPaddingBottom) {
            lr.computedBox.height += *resolvedPaddingBottom;
        }

        lr.computedPaddingBox = lr.computedBox;

        lr.computedInnerBox = {
            minContentX,
            minY,
            maxContentX - minContentX,
            totalHeight
        };

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

    std::optional<IntrinsicSizes> inlinePass(const InlineState& state, const SizeRequest& sizeRequest) {
        std::optional<IntrinsicSizes> intrinsicResult;

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
