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
#include <algorithm>
#include <optional>
#include <simd/vector_types.h>


namespace layout {
    BlockState LayoutEngine::layoutBlockOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {

        BlockState lr;
        lr.outOfFlow = true;

        auto margins = constraints.resolvedMargins;

        ContainingBlock containingBlock =
            layoutInput.position == Position::Fixed
                ? ContainingBlock {
                    .origin = {0.0f, 0.0f},
                    .width = constraints.frameInfo.width,
                    .height = constraints.frameInfo.height
                }
                : constraints.absoluteContainingBlock;

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
        }, sizeResult.outerSize.width);

        float computedHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.outerSize.height);
        lr.computedBox = {
            .x = position.x,
            .y = position.y,
            .width = computedWidth,
            .height = computedHeight
        };

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

        // Defer right/bottom positioning to postLayout where final sizes are known
        bool isRtl = constraints.inheritedProperties.direction == Direction::rtl;

        lr.deferredPosition = {
            .containingBlockWidth = containingBlock.width,
            .containingBlockHeight = containingBlock.height,
            .right = (!layoutInput.left.has_value() || isRtl) ? layoutInput.right : std::nullopt,
            .bottom = !layoutInput.top.has_value() ? layoutInput.bottom : std::nullopt,
            .marginRight = margins.right,
            .marginBottom = margins.bottom
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
        }, sizeResult.outerSize.width);

        float computedHeight = std::visit(Overloaded{
            [&](float resolved){ return resolved; },
            [&](auto&) { return 0.0f; }
        }, sizeResult.outerSize.height);

        lr.computedBox = {
            startingPos.x,
            startingPos.y,
            computedWidth,
            computedHeight
        };

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

}
