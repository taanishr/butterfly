//
//  block.cpp
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
    BlockState LayoutEngine::layoutBlockOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {

        BlockState lr;
        lr.outOfFlow = true;

        auto margins = constraints.resolvedMargins;

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .sizeResult = sizeResult,
            .margins = margins
        };

        simd_float2 position = resolvePosition(pctx);

        float computedWidth = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.borderBoxSize.width);

        float computedHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.borderBoxSize.height);
        lr.computedBox = {
            .x = position.x,
            .y = position.y,
            .width = computedWidth,
            .height = computedHeight
        };

        float computedPaddingBoxWidth = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.paddingBoxSize.width);

        float computedPaddingBoxHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.paddingBoxSize.height);

        float computedInnerWidth = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.innerSize.width);

        float computedInnerHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.innerSize.height);

        const auto* resolvedBorderLeft = std::get_if<float>(&sizeResult.border.left);
        const auto* resolvedBorderTop = std::get_if<float>(&sizeResult.border.top);
        const auto* resolvedPaddingLeft = std::get_if<float>(&sizeResult.padding.left);
        const auto* resolvedPaddingTop = std::get_if<float>(&sizeResult.padding.top);

        lr.computedPaddingBox = {
            .x = lr.computedBox.x,
            .y = lr.computedBox.y,
            .width = computedPaddingBoxWidth,
            .height = computedPaddingBoxHeight
        };

        if (resolvedBorderLeft) {
            lr.computedPaddingBox.x += *resolvedBorderLeft;
        }

        if (resolvedBorderTop) {
            lr.computedPaddingBox.y += *resolvedBorderTop;
        }

        lr.computedInnerBox = {
            .x = lr.computedPaddingBox.x,
            .y = lr.computedPaddingBox.y,
            .width = computedInnerWidth,
            .height = computedInnerHeight
        };

        if (resolvedPaddingLeft) {
            lr.computedInnerBox.x += *resolvedPaddingLeft;
        }

        if (resolvedPaddingTop) {
            lr.computedInnerBox.y += *resolvedPaddingTop;
        }

        lr.atomOffsets = {
            position
        };
        lr.siblingCursor = currentCursor;

        lr.childConstraints = {
            .origin = {0, 0},
            .cursor = {0, 0},
            .availableWidth = sizeResult.innerSize.width,
            .availableHeight = sizeResult.innerSize.height,
            .frameInfo = constraints.frameInfo
        };

        return lr;
    }

    // relative, block/inline
    BlockState LayoutEngine::layoutBlockNormalFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        BlockState lr;
        Constraints childConstraints;

        lr.outOfFlow = false;

        auto margins = constraints.resolvedMargins;

        PositionResolutionContext pctx {
            .currentCursor = currentCursor,
            .constraints = constraints,
            .layoutInput = layoutInput,
            .sizeResult = sizeResult,
            .margins = margins
        };
        simd_float2 startingPos = resolvePosition(pctx);

        simd_float2 newCursor {startingPos};
        
        // local cursor
        childConstraints.cursor.x = 0;
        childConstraints.cursor.y = 0;

        // globalish cursor?
        childConstraints.origin = childConstraints.cursor;

        // frame info
        childConstraints.frameInfo = constraints.frameInfo;

        // why does this default to 0.0f?
        // The sizing request is the canonical source of what size something is
        // or if it doesnt have a size, why it isnt sized
        // Meanwhile, the computed box answers: what does the box look like at this moment
        // for a box that is unresolved, it looks like *nothing*
        float computedWidth = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.borderBoxSize.width);

        float computedHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.borderBoxSize.height);

        lr.computedBox = {
            startingPos.x,
            startingPos.y,
            computedWidth,
            computedHeight
        };

        float computedPaddingBoxWidth = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.paddingBoxSize.width);

        float computedPaddingBoxHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.paddingBoxSize.height);

        float computedInnerWidth = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.innerSize.width);

        float computedInnerHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.innerSize.height);

        const auto* resolvedBorderLeft = std::get_if<float>(&sizeResult.border.left);
        const auto* resolvedBorderTop = std::get_if<float>(&sizeResult.border.top);
        const auto* resolvedPaddingLeft = std::get_if<float>(&sizeResult.padding.left);
        const auto* resolvedPaddingTop = std::get_if<float>(&sizeResult.padding.top);

        lr.computedPaddingBox = {
            .x = lr.computedBox.x,
            .y = lr.computedBox.y,
            .width = computedPaddingBoxWidth,
            .height = computedPaddingBoxHeight
        };

        if (resolvedBorderLeft) {
            lr.computedPaddingBox.x += *resolvedBorderLeft;
        }

        if (resolvedBorderTop) {
            lr.computedPaddingBox.y += *resolvedBorderTop;
        }

        lr.computedInnerBox = {
            .x = lr.computedPaddingBox.x,
            .y = lr.computedPaddingBox.y,
            .width = computedInnerWidth,
            .height = computedInnerHeight
        };

        if (resolvedPaddingLeft) {
            lr.computedInnerBox.x += *resolvedPaddingLeft;
        }

        if (resolvedPaddingTop) {
            lr.computedInnerBox.y += *resolvedPaddingTop;
        }

        lr.atomOffsets = {
            startingPos
        };

        childConstraints.availableWidth = sizeResult.innerSize.width;
        childConstraints.availableHeight = sizeResult.innerSize.height;
        
        lr.childConstraints = childConstraints;

        newCursor.y += lr.computedBox.height;
        newCursor.x = constraints.origin.x;

        lr.siblingCursor = newCursor;

        // Auto margins don't participate in margin collapse
        lr.edgeIntent = {
            .edgeDisplayMode = Display::Block,
            .intent = margins.bottom,
            .collapsable = !layoutInput.marginBottom.isAuto(),
        };

        return lr;
    }

    std::optional<IntrinsicSizes> blockPass(
        tree::RenderTree& tree, tree::TreeNode* node, const FrameInfo& frameInfo, BlockState& state,
        Constraints childConstraints, const SizeRequest& sizeRequest, const SizeResult& sizeResult,
        bool mutate, std::unordered_map<size_t, SizeResult>& sizeCache
    ) {
        std::optional<IntrinsicSizes> intrinsicResult;

        if (sizeRequest.resolvingIntrinsicWidth || sizeRequest.resolvingIntrinsicHeight) {
            intrinsicResult = IntrinsicSizes{};
        }

        InlineSizingInput inlineSizing {
            .availableWidth = childConstraints.availableWidth,
            .widthRequest = sizeRequest.intrinsicWidthRequest,
            .trackIntrinsicWidth = sizeRequest.resolvingIntrinsicWidth,
        };

        // right now, minimum & maximum content dont really get set?
        // it only changes for flex/grid/etc...
        // which provide different contributions not based on intrinsic size collection but
        // min and max bounds; this needs to be fixed
        auto inlineFormatting = tree::buildInlineBoxes(tree, node, frameInfo, childConstraints, sizeRequest, inlineSizing, sizeCache);

        if (inlineFormatting->intrinsicSizes) {
            intrinsicResult = *inlineFormatting->intrinsicSizes;
        }

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto child = node->children[i].get();

            childConstraints.inlineFormatting = {
                .context = inlineFormatting,
                .fragments = inlineFormatting->childFragments[i],
                .minFragments = inlineFormatting->minChildFragments.empty()
                    ? InlineFragmentRange{}
                    : inlineFormatting->minChildFragments[i],
                .maxFragments = inlineFormatting->maxChildFragments.empty()
                    ? InlineFragmentRange{}
                    : inlineFormatting->maxChildFragments[i],
            };

            auto childOutput = tree.layoutRecursive(child, frameInfo, childConstraints, mutate, std::nullopt, sizeRequest.intrinsicWidthRequest, sizeRequest.intrinsicHeightRequest);

            std::visit([&](const auto& childLayout) {
            if (!childLayout.outOfFlow) {
                if (sizeRequest.resolvingIntrinsicWidth) {
                    const auto& margins = child->preLayout->resolvedMargins;
                    intrinsicResult->minimum = std::max(intrinsicResult->minimum, margins.left + childOutput.intrinsicSizes->minimum + margins.right);
                    intrinsicResult->maximum = std::max(intrinsicResult->maximum, margins.left + childOutput.intrinsicSizes->maximum + margins.right);
                }

                if (sizeRequest.resolvingIntrinsicHeight) {
                    float childOffset = childConstraints.cursor.y - childConstraints.origin.y;
                    intrinsicResult->minimum = std::max(intrinsicResult->minimum, childOffset + childOutput.intrinsicSizes->minimum);
                    intrinsicResult->maximum = std::max(intrinsicResult->maximum, childOffset + childOutput.intrinsicSizes->maximum);
                }

                childConstraints.cursor = childLayout.siblingCursor;
                childConstraints.edgeIntent = childLayout.edgeIntent;
                childConstraints.prevInlineHeight = childLayout.prevInlineHeight;

            }
            }, childOutput.layout);
        }

        if (sizeRequest.resolvingIntrinsicWidth) {
            return resolveContributionWidth(*intrinsicResult, sizeResult);
        }

        if (sizeRequest.resolvingIntrinsicHeight) {
            IntrinsicSizes contribution = resolveContributionHeight(*intrinsicResult, sizeResult);

            if (!state.outOfFlow && !std::holds_alternative<float>(sizeResult.borderBoxSize.height)) {
                state.siblingCursor.y += contribution.maximum;
            }

            return contribution;
        }

        return intrinsicResult;
    }

}
