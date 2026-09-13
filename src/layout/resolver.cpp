//
//  resolver.cpp
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

    simd_float2 resolvePosition(const PositionResolutionContext& ctx) {
        simd_float2 resolvedPosition = ctx.currentCursor;

        switch (ctx.layoutInput.position) {
            case layout::Position::Fixed: {
                float refWidth = ctx.constraints.frameInfo.width;
                float refHeight = ctx.constraints.frameInfo.height;

                std::optional<float> left;
                if (ctx.layoutInput.left.has_value()) {
                    auto resolvedLeft = ctx.layoutInput.left->resolve(Size::px(refWidth));
                    if (resolvedLeft) left = *resolvedLeft;
                }
                std::optional<float> top;
                if (ctx.layoutInput.top.has_value()) {
                    auto resolvedTop = ctx.layoutInput.top->resolve(Size::px(refHeight));
                    if (resolvedTop) top = *resolvedTop;
                }

                resolvedPosition = {0.0f, 0.0f};

                // Only resolve left/top during layout.
                // right/bottom depend on element size and are resolved in postLayout.
                if (left.has_value()) {
                    resolvedPosition.x = *left + ctx.margins.left;
                }
                if (top.has_value()) {
                    resolvedPosition.y = *top + ctx.margins.top;
                }

                break;
            }
            case layout::Position::Absolute: {
                auto& cb = ctx.constraints.absoluteContainingBlock;

                std::optional<float> left;
                if (ctx.layoutInput.left.has_value()) {
                    SizeState resolvedLeft = calculateSize(*ctx.layoutInput.left, cb.width);
                    if (std::holds_alternative<float>(resolvedLeft)) {
                        left = std::get<float>(resolvedLeft);
                    }
                }
                std::optional<float> top;
                if (ctx.layoutInput.top.has_value()) {
                    SizeState resolvedTop = calculateSize(*ctx.layoutInput.top, cb.height);
                    if (std::holds_alternative<float>(resolvedTop)) {
                        top = std::get<float>(resolvedTop);
                    }
                }

                resolvedPosition = {0.0f, 0.0f};

                // Only resolve left/top during layout.
                // right/bottom depend on element size and are resolved in postLayout.
                if (left.has_value()) {
                    resolvedPosition.x = *left + ctx.margins.left;
                }
                if (top.has_value()) {
                    resolvedPosition.y = *top + ctx.margins.top;
                }

                break;
            }   
            case layout::Position::Relative:
            case layout::Position::Static: {
                switch (ctx.layoutInput.display) {
                    case layout::Display::Flex:
                    case layout::Display::Grid:
                    case layout::Display::Block: {
                        float startingX = ctx.constraints.origin.x;
                        float startingY = ctx.constraints.cursor.y;

                        if (ctx.constraints.edgeIntent.edgeDisplayMode == Display::Inline) {
                            startingY += ctx.constraints.prevInlineHeight;
                        }else if (ctx.constraints.edgeIntent.edgeDisplayMode == Display::Block) {
                            if (ctx.constraints.edgeIntent.collapsable && !ctx.layoutInput.marginTop.isAuto()) {
                                startingY += std::max(ctx.constraints.edgeIntent.intent, ctx.margins.top);
                            } else {
                                startingY += ctx.constraints.edgeIntent.intent + ctx.margins.top;
                            }
                        }

                        if (ctx.constraints.inheritedProperties.direction == Direction::ltr) {
                            startingX += ctx.margins.left;
                        } else {
                            if (std::holds_alternative<float>(ctx.constraints.availableWidth)) {
                                float availableWidth = std::get<float>(ctx.constraints.availableWidth);
                                float width = std::holds_alternative<float>(ctx.sizeResult.outerSize.width)
                                    ? std::get<float>(ctx.sizeResult.outerSize.width)
                                    : availableWidth;
                                startingX = ctx.constraints.origin.x + availableWidth - width - ctx.margins.right;
                            }
                        }

                        resolvedPosition = {startingX, startingY};
                        break;
                    }
                    case layout::Display::Inline: {
                        resolvedPosition = ctx.currentCursor;
                        break;
                    }
                    default:
                        break;
                }

                // Relative: apply offsets after computing static position
                if (ctx.layoutInput.position == Position::Relative) {
                    if (ctx.layoutInput.top.has_value()) {
                        SizeState top = calculateSize(*ctx.layoutInput.top, ctx.constraints.availableHeight);
                        if (std::holds_alternative<float>(top)) {
                            resolvedPosition.y += std::get<float>(top);
                        }
                    } else if (ctx.layoutInput.bottom.has_value()) {
                        SizeState bottom = calculateSize(*ctx.layoutInput.bottom, ctx.constraints.availableHeight);
                        if (std::holds_alternative<float>(bottom)) {
                            resolvedPosition.y -= std::get<float>(bottom);
                        }
                    }

                    if (ctx.layoutInput.left.has_value()) {
                        SizeState left = calculateSize(*ctx.layoutInput.left, ctx.constraints.availableWidth);
                        if (std::holds_alternative<float>(left)) {
                            resolvedPosition.x += std::get<float>(left);
                        }
                    } else if (ctx.layoutInput.right.has_value()) {
                        SizeState right = calculateSize(*ctx.layoutInput.right, ctx.constraints.availableWidth);
                        if (std::holds_alternative<float>(right)) {
                            resolvedPosition.x -= std::get<float>(right);
                        }
                    }
                }

                break;
            }
            default:
                break;
        }

        return resolvedPosition;
    }

    // Resolve auto margins for centering
    ResolvedMargins LayoutEngine::resolveAutoMargins(
        const LayoutInput& li,
        const ReplacedAttributes& replacedAttributes,
        const SizeState& availableWidth,
        float contentWidth
    ) {
        ResolvedMargins margins;

        // Vertical margins: auto resolves to 0
        margins.bottom = li.marginBottom.resolveOr(Size::px(0.0f), 0.0f);

        if (replacedAttributes.marginTop.has_value()) {
            auto& replacedMarginTop = *replacedAttributes.marginTop;
            margins.top = replacedMarginTop.resolveOr(Size::px(0.0f), 0.0f);
        }
        else {
            margins.top = li.marginTop.resolveOr(Size::px(0.0f), 0.0f);
        }

        if (replacedAttributes.marginBottom.has_value()) {
            auto replacedMarginBottom = *replacedAttributes.marginBottom;
            margins.bottom = replacedMarginBottom.resolveOr(Size::px(0.0f), 0.0f);
        }
        else {
            margins.bottom = li.marginBottom.resolveOr(Size::px(0.0f), 0.0f);
        }

        // Horizontal margins: check for auto centering
        bool leftAuto = li.marginLeft.isAuto();
        bool rightAuto = li.marginRight.isAuto();

        SizeState calculatedAvailableWidth = calculateSize(availableWidth, std::monostate{});
        SizeState calculatedMarginLeft = calculateSize(li.marginLeft, availableWidth);
        SizeState calculatedMarginRight = calculateSize(li.marginRight, availableWidth);

        float resolvedAvailableWidth = std::holds_alternative<float>(calculatedAvailableWidth)
            ? std::get<float>(calculatedAvailableWidth)
            : 0.0f;
        float resolvedMarginLeft = std::holds_alternative<float>(calculatedMarginLeft)
            ? std::get<float>(calculatedMarginLeft)
            : 0.0f;
        float resolvedMarginRight = std::holds_alternative<float>(calculatedMarginRight)
            ? std::get<float>(calculatedMarginRight)
            : 0.0f;

        if (leftAuto && rightAuto) {
            // Both auto: center horizontally
            float remainingSpace = resolvedAvailableWidth - contentWidth;
            if (remainingSpace > 0) {
                float autoMargin = remainingSpace / 2.0f;
                margins.left = autoMargin;
                margins.right = autoMargin;
            } else {
                // No space: auto resolves to 0
                margins.left = 0.0f;
                margins.right = 0.0f;
            }
        } else if (leftAuto) {
            // Only left auto: push to right (absorb remaining space)
            margins.right = resolvedMarginRight;
            float remainingSpace = resolvedAvailableWidth - contentWidth - margins.right;
            margins.left = std::max(0.0f, remainingSpace);
        } else if (rightAuto) {
            // Only right auto: resolves to 0 (default left alignment)
            margins.left = resolvedMarginLeft;
            margins.right = 0.0f;
        } else {
            // Neither auto: resolve normally
            margins.left = resolvedMarginLeft;
            margins.right = resolvedMarginRight;
        }

        return margins;
    }

}

namespace layout {
    LayoutState LayoutEngine::resolveOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        if (layoutInput.display == Display::Block || layoutInput.display == Display::Flex || layoutInput.display == Display::Grid) {
            return layoutBlockOutOfFlow(constraints, currentCursor, layoutInput, atomized, sizeResult);
        }

        return layoutInlineOutOfFlow(constraints, currentCursor, layoutInput, atomized, sizeResult);
    }

    LayoutState LayoutEngine::resolveNormalFlow(Constraints& constraints, simd_float2 current_cursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        if (layoutInput.display == Display::Block || layoutInput.display == Display::Flex || layoutInput.display == Display::Grid) {
            return layoutBlockNormalFlow(constraints, current_cursor, layoutInput, atomized, sizeResult);
        }

        return layoutInlineNormalFlow(constraints, current_cursor, layoutInput, atomized, sizeResult);
    }

    LayoutState LayoutEngine::resolve(Constraints& constraints, LayoutInput& layoutInput, Atomized atomized, const SizeResult& sizeResult)
    {
        if (layoutInput.direction.has_value()) {
            constraints.inheritedProperties.direction = *layoutInput.direction;
        }
        if (layoutInput.textAlign.has_value()) {
            constraints.inheritedProperties.textAlign = *layoutInput.textAlign;
        }
        
        
        if (layoutInput.position == Position::Fixed || layoutInput.position == Position::Absolute) {
            return resolveOutOfFlow(constraints, constraints.cursor, layoutInput, atomized, sizeResult);
        }

        return resolveNormalFlow(constraints, constraints.cursor, layoutInput, atomized, sizeResult);
    }
}
