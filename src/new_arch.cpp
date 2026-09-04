//
//  new_arch.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "new_arch.hpp"
#include "fragment_types.hpp"
#include "overloaded.hpp"
#include "sizing.hpp"
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
                    auto resolvedLeft = ctx.layoutInput.left->resolve(cb.width);
                    if (resolvedLeft) left = *resolvedLeft;
                }
                std::optional<float> top;
                if (ctx.layoutInput.top.has_value()) {
                    auto resolvedTop = ctx.layoutInput.top->resolve(cb.height);
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
                            if (!ctx.constraints.availableWidth.isAuto()) {
                                float width = std::holds_alternative<float>(ctx.sizeResult.outerSize.width)
                                    ? std::get<float>(ctx.sizeResult.outerSize.width)
                                    : ctx.constraints.availableWidth.value;
                                startingX = ctx.constraints.origin.x + ctx.constraints.availableWidth.value - width - ctx.margins.right;
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
                        resolvedPosition.y += ctx.layoutInput.top->resolveOr(ctx.constraints.availableHeight, 0.0f);
                    } else if (ctx.layoutInput.bottom.has_value()) {
                        resolvedPosition.y -= ctx.layoutInput.bottom->resolveOr(ctx.constraints.availableHeight, 0.0f);
                    }

                    if (ctx.layoutInput.left.has_value()) {
                        resolvedPosition.x += ctx.layoutInput.left->resolveOr(ctx.constraints.availableWidth, 0.0f);
                    } else if (ctx.layoutInput.right.has_value()) {
                        resolvedPosition.x -= ctx.layoutInput.right->resolveOr(ctx.constraints.availableWidth, 0.0f);
                    }
                }

                break;
            }
            default:
                break;
        }

        return resolvedPosition;
    }

    ResolvedSize resolveSize(const SizeResolutionContext& ctx)
    {
        ResolvedSize resolvedSize;

        switch (ctx.position) {
            case layout::Position::Absolute:
            case layout::Position::Fixed: {
                resolvedSize.height =
                    ctx.requestedHeight.resolve(ctx.availableHeight);
                if (!resolvedSize.height &&
                    resolvedSize.height.error() == style::SizeError::Auto) {
                    std::optional<float> resolvedTop;
                    std::optional<float> resolvedBottom;

                    if (ctx.top.has_value() && !ctx.availableHeight.isAuto()) {
                        resolvedTop = ctx.top->resolveOr(ctx.availableHeight, 0.0f);
                    }

                    if (ctx.bottom.has_value() && !ctx.availableHeight.isAuto()) {
                        resolvedBottom = ctx.bottom->resolveOr(ctx.availableHeight, 0.0f);
                    }

                    if (resolvedTop.has_value() && resolvedBottom.has_value()) {
                        resolvedSize.height = ctx.availableHeight.value - *resolvedTop - *resolvedBottom;
                    }
                }

                resolvedSize.width =
                    ctx.requestedWidth.resolve(ctx.availableWidth);
                if (!resolvedSize.width &&
                    resolvedSize.width.error() == style::SizeError::Auto) {
                    std::optional<float> resolvedRight;
                    std::optional<float> resolvedLeft;

                    if (ctx.right.has_value() && !ctx.availableWidth.isAuto()) {
                        resolvedRight = ctx.right->resolveOr(ctx.availableWidth, 0.0f);
                    }

                    if (ctx.left.has_value() && !ctx.availableWidth.isAuto()) {
                        resolvedLeft = ctx.left->resolveOr(ctx.availableWidth, 0.0f);
                    }

                    if (resolvedRight.has_value() && resolvedLeft.has_value()) {
                        resolvedSize.width = ctx.availableWidth.value - *resolvedRight - * resolvedLeft;
                    }
                }

                break;
            }
            default: {
                resolvedSize.height =
                    ctx.requestedHeight.resolve(ctx.availableHeight);
                resolvedSize.width =
                    ctx.requestedWidth.resolve(ctx.availableWidth);
            }
        };

        return resolvedSize;
    }

    void transferAspectRatio(
        std::expected<float, style::SizeError>& width,
        std::expected<float, style::SizeError>& height,
        float ratio
    ) {
        if (ratio <= 0.0f) return;

        if (width && !height) {
            height = *width / ratio;
        } else if (height && !width) {
            width = *height * ratio;
        }
    }

    // Resolve auto margins for centering
    ResolvedMargins LayoutEngine::resolveAutoMargins(
        const LayoutInput& li,
        const ReplacedAttributes& replacedAttributes,
        Size availableWidth,
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

        if (leftAuto && rightAuto) {
            // Both auto: center horizontally
            float remainingSpace = availableWidth.isAuto() ? 0.0f : availableWidth.value - contentWidth;
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
            margins.right = li.marginRight.resolveOr(availableWidth, 0.0f);
            float remainingSpace = availableWidth.isAuto() ? 0.0f : availableWidth.value - contentWidth - margins.right;
            margins.left = std::max(0.0f, remainingSpace);
        } else if (rightAuto) {
            // Only right auto: resolves to 0 (default left alignment)
            margins.left = li.marginLeft.resolveOr(availableWidth, 0.0f);
            margins.right = 0.0f;
        } else {
            // Neither auto: resolve normally
            margins.left = li.marginLeft.resolveOr(availableWidth, 0.0f);
            margins.right = li.marginRight.resolveOr(availableWidth, 0.0f);
        }

        return margins;
    }

    ResolvedMargins resolveMargins(
        const LayoutInput& li
    ) {

        float marginTop = li.marginTop.resolveOr(Size::px(0.0f), 0.0f);
        float marginRight = li.marginRight.resolveOr(Size::px(0.0f), 0.0f);
        float marginBottom = li.marginBottom.resolveOr(Size::px(0.0f), 0.0f);
        float marginLeft = li.marginLeft.resolveOr(Size::px(0.0f), 0.0f);

        ResolvedMargins resolvedMargins {
            .top = marginTop,
            .right = marginRight,
            .bottom = marginBottom,
            .left = marginLeft,
        };

        return resolvedMargins;
    }

}

namespace runtime {

    // pipeline specific
    UIContext::UIContext(MTL::Device* device, MTK::View* view):
        device{device},
        view{view},
        allocator{DrawableBufferAllocator{device}},
        layoutEngine{},
        frameInfoBuffer{allocator.allocate(sizeof(FrameInfo))},
        frameIndex{0}
    {
        auto frameDimensions = this->view->drawableSize();
        auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(view));

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};

        this->frameInfo = frameInfo;
        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    };

    void UIContext::updateView() {
        auto frameDimensions = this->view->drawableSize();

        auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(view));

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};
        this->frameInfo = frameInfo;

        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    }

}

namespace layout {
    LayoutState LayoutEngine::layoutBlockOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {

        LayoutState lr;
        lr.outOfFlow = true;

        auto margins = constraints.resolvedMargins;

        ContainingBlock containingBlock =
            layoutInput.position == Position::Fixed
                ? ContainingBlock {
                    .origin = {0.0f, 0.0f},
                    .width = Size::px(constraints.frameInfo.width),
                    .height = Size::px(constraints.frameInfo.height)
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

        lr.resolvedSize.width = layoutInput.width;
        lr.resolvedSize.height = layoutInput.height;

        float resolvedWidth = lr.resolvedSize.width.value_or(0.0f);
        float resolvedHeight = lr.resolvedSize.height.value_or(0.0f);


        lr.computedBox = {
            .x = position.x,
            .y = position.y,
            .width = resolvedWidth,
            .height = resolvedHeight
        };

        lr.atomOffsets = {
            position
        };
        lr.siblingCursor = currentCursor;

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.availableWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.availableHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.availableWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.availableHeight);

        lr.childConstraints = {
            .origin = {0, 0},
            .cursor = {0, 0},
            .availableWidth = layoutInput.width.has_value() ? Size::px(lr.computedBox.width - paddingLeft - paddingRight) : Size::autoSize(),
            .availableHeight = layoutInput.height.has_value() ? Size::px(lr.computedBox.height - paddingTop - paddingBottom) : Size::autoSize(),
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

    LayoutState LayoutEngine::resolveOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        LayoutState lr;

        if (layoutInput.display == Display::Block || layoutInput.display == Display::Flex || layoutInput.display == Display::Grid) {
            lr = layoutBlockOutOfFlow(constraints, currentCursor, layoutInput, atomized, sizeResult);
        }else {
            lr = layoutInlineOutOfFlow(constraints, currentCursor, layoutInput, atomized, sizeResult);
        }

        return lr;
    }

    // relative, block/inline
    LayoutState LayoutEngine::layoutBlockNormalFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        LayoutState lr;
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

        // these are now dead; only user is grid (which will change slowly)
        // // these need to die lol
        // lr.resolvedSize.width = computedWidth;
        // lr.resolvedSize.height = computedHeight;

        lr.computedBox = {
            startingPos.x,
            startingPos.y,
            computedWidth,
            computedHeight
        };

        lr.atomOffsets = {
            startingPos
        };

        childConstraints.availableWidth = std::visit(Overloaded{
            [&](float resolved){ return Size::px(resolved); },
            [&](auto&) { return Size::autoSize(); }
        }, sizeResult.innerSize.width);
    
        childConstraints.availableHeight = std::visit(Overloaded{
            [&](float resolved){ return Size::px(resolved); },
            [&](auto&) { return Size::autoSize(); }
        }, sizeResult.innerSize.height);
        
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

    void LineBox::pushFragment(const LineFragment& fragment) {
        fragmentOffsets.push_back(currentFragmentOffset);
        currentFragmentOffset += fragment.width;
        width += fragment.width;
        fragmentCount += 1;
    }

    LayoutState LayoutEngine::layoutInlineNormalFlow(
        Constraints& constraints,
        simd_float2 currentCursor,
        LayoutInput& layoutInput,
        Atomized& atomized,
        const SizeResult& sizeResult
    ) {
        LayoutState lr;
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

        lr.childConstraints.origin = {0.0f, 0.0f};
        float lineHeight = 0;
        float totalHeight = 0;
        float totalWidth = 0;
        float currentTotalWidth = 0;
        float minX = newCursor.x;
        float minY = newCursor.y;

        bool isLtr = constraints.inheritedProperties.direction == Direction::ltr;
        size_t prevLineBoxIndex = -1;

        auto lineFragments = constraints.inlineFormatting.lineFragments();
        auto lineBoxes = constraints.inlineFormatting.lineBoxes();
        size_t fragmentIdx = 0;
        for (auto it = lineFragments.begin(); it != lineFragments.end(); ++it, ++fragmentIdx) {
            const LineFragment& fragment = *it;

            const auto& lineBox = lineBoxes[fragment.lineBoxIndex];
            float offset = lineBox.fragmentOffsets[fragment.fragmentIndex];
            float lineOffset = 0.0f;
            switch (constraints.inheritedProperties.textAlign) {
                case TextAlign::Start:
                    lineOffset = isLtr || constraints.availableWidth.isAuto() ? 0.0f : constraints.availableWidth.value - lineBox.width;
                    break;
                case TextAlign::Left:
                    break;
                case TextAlign::Center:
                    lineOffset = constraints.availableWidth.isAuto() ? 0.0f : (constraints.availableWidth.value - lineBox.width) / 2.0f;
                    break;
                case TextAlign::Right:
                    lineOffset = constraints.availableWidth.isAuto() ? 0.0f : constraints.availableWidth.value - lineBox.width;
                    break;
            }
            float startingX = constraints.origin.x + lineOffset + offset;

            newCursor.x = startingX;
            
            minX = std::min(minX, newCursor.x);

            if (fragmentIdx == 0) {

                float inlineMargin = isLtr ? margins.left : margins.right;

                if (constraints.edgeIntent.edgeDisplayMode == Display::Inline) {
                    if (constraints.edgeIntent.collapsable) {
                        float collapsed = std::max(inlineMargin, constraints.edgeIntent.intent);
                        newCursor.x += collapsed;
                    } else {
                        float total = inlineMargin + constraints.edgeIntent.intent;
                        newCursor.x += total;
                    }
                } else {
                    if (constraints.edgeIntent.collapsable) {
                        newCursor.y += std::max(margins.top, constraints.edgeIntent.intent);
                    } else {
                        newCursor.y += margins.top + constraints.edgeIntent.intent;
                    }

                    newCursor.x += (isLtr ? inlineMargin : -inlineMargin);
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
                float usedLineHeight = atom.lineHeight > 0.0f
                    ? atom.lineHeight
                    : atom.height;

                atomOffsets[atomIndex] = newCursor + simd_float2{
                    0.0f,
                    (usedLineHeight - atom.height) / 2.0f
                };
                newCursor.x += atom.width;
                lineHeight = std::max(lineHeight, usedLineHeight);
                currentTotalWidth += atom.width;
            }

            prevLineBoxIndex = fragment.lineBoxIndex;
        }

        newCursor.x += isLtr ? margins.right : -margins.left;

        totalHeight += lineHeight;
        totalWidth = std::max(currentTotalWidth, totalWidth);

        lr.resolvedSize.width = totalWidth;
        lr.resolvedSize.height = totalHeight;

        lr.computedBox = {
            minX,
            minY,
            totalWidth,
            totalHeight
        };

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.availableWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.availableHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.availableWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.availableHeight);

        lr.childConstraints.cursor = {0, 0};
        lr.childConstraints.availableWidth = Size::px(totalWidth - paddingLeft - paddingRight);
        lr.childConstraints.availableHeight = Size::px(totalHeight - paddingTop - paddingBottom);
        lr.childConstraints.frameInfo = constraints.frameInfo;

        lr.atomOffsets = atomOffsets;
        lr.prevInlineHeight = lineHeight;

        lr.siblingCursor = newCursor;

        lr.edgeIntent = {
            .edgeDisplayMode = Display::Inline,
            .intent = isLtr ? margins.right : margins.left,
            .collapsable = false,
        };

        return lr;
    }

    LayoutState LayoutEngine::layoutInlineOutOfFlow(Constraints& constraints, simd_float2 currentCursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        LayoutState lr;
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

        lr.childConstraints.origin = {0.0f, 0.0f};
        float lineHeight = 0;
        float totalHeight = 0;
        float totalWidth = 0;
        float currentTotalWidth = 0;
        float minX = newCursor.x;
        float minY = newCursor.y;

        bool isLtr = constraints.inheritedProperties.direction == Direction::ltr;
        size_t prevLineBoxIndex = -1;

        auto lineFragments = constraints.inlineFormatting.lineFragments();
        auto lineBoxes = constraints.inlineFormatting.lineBoxes();
        for (auto it = lineFragments.begin(); it != lineFragments.end(); ++it) {
            const LineFragment& fragment = *it;

            const auto& lineBox = lineBoxes[fragment.lineBoxIndex];
            float offset = lineBox.fragmentOffsets[fragment.fragmentIndex];
            float lineOffset = 0.0f;
            switch (constraints.inheritedProperties.textAlign) {
                case TextAlign::Start:
                    lineOffset = isLtr || constraints.availableWidth.isAuto() ? 0.0f : constraints.availableWidth.value - lineBox.width;
                    break;
                case TextAlign::Left:
                    break;
                case TextAlign::Center:
                    lineOffset = constraints.availableWidth.isAuto() ? 0.0f : (constraints.availableWidth.value - lineBox.width) / 2.0f;
                    break;
                case TextAlign::Right:
                    lineOffset = constraints.availableWidth.isAuto() ? 0.0f : constraints.availableWidth.value - lineBox.width;
                    break;
            }
            float startingX = originX + lineOffset + offset;

            newCursor.x = startingX;

            minX = std::min(minX, newCursor.x);

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
                currentTotalWidth += atom.width;
            }

            prevLineBoxIndex = fragment.lineBoxIndex;
        }

        totalHeight += lineHeight;
        totalWidth = std::max(currentTotalWidth, totalWidth);

        lr.resolvedSize.width = totalWidth;
        lr.resolvedSize.height = totalHeight;

        lr.computedBox = {
            minX,
            minY,
            totalWidth,
            totalHeight
        };

        float paddingLeft = layoutInput.paddingLeft.resolveOr(constraints.availableWidth);
        float paddingTop = layoutInput.paddingTop.resolveOr(constraints.availableHeight);
        float paddingRight = layoutInput.paddingRight.resolveOr(constraints.availableWidth);
        float paddingBottom = layoutInput.paddingBottom.resolveOr(constraints.availableHeight);

        lr.childConstraints.cursor = {0, 0};
        lr.childConstraints.availableWidth = Size::px(totalWidth - paddingLeft - paddingRight);
        lr.childConstraints.availableHeight = Size::px(totalHeight - paddingTop - paddingBottom);
        lr.childConstraints.frameInfo = constraints.frameInfo;

        lr.atomOffsets = atomOffsets;

        lr.siblingCursor = currentCursor;

        ContainingBlock containingBlock =
            layoutInput.position == Position::Fixed
                ? ContainingBlock {
                    .origin = {0.0f, 0.0f},
                    .width = Size::px(constraints.frameInfo.width),
                    .height = Size::px(constraints.frameInfo.height)
                }
                : constraints.absoluteContainingBlock;

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


    LayoutState LayoutEngine::resolveNormalFlow(Constraints& constraints, simd_float2 current_cursor, LayoutInput& layoutInput, Atomized& atomized, const SizeResult& sizeResult) {
        LayoutState lr;

        if (layoutInput.display == Display::Block || layoutInput.display == Display::Flex || layoutInput.display == Display::Grid) {
            lr = layoutBlockNormalFlow(constraints, current_cursor, layoutInput, atomized, sizeResult);
        }else {
            lr = layoutInlineNormalFlow(constraints, current_cursor, layoutInput, atomized, sizeResult);
        }

        return lr;
    }

    LayoutState LayoutEngine::resolve(Constraints& constraints, LayoutInput& layoutInput, Atomized atomized, const SizeResult& sizeResult)
    {
        LayoutState lr;

        if (layoutInput.direction.has_value()) {
            constraints.inheritedProperties.direction = *layoutInput.direction;
        }
        if (layoutInput.textAlign.has_value()) {
            constraints.inheritedProperties.textAlign = *layoutInput.textAlign;
        }
        
        
        if (layoutInput.position == Position::Fixed || layoutInput.position == Position::Absolute) {
            lr = resolveOutOfFlow(constraints, constraints.cursor, layoutInput, atomized, sizeResult);
        }else {
            lr = resolveNormalFlow(constraints, constraints.cursor, layoutInput, atomized, sizeResult);
        }
        
        return lr;
    }
}
