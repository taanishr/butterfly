#include "flex.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "render_tree.hpp"
#include <print>
#include <optional>

namespace layout {
    Alignment distributeSpace(float remainingSpace, size_t itemCount, DistributeMode mode) {
        Alignment a;
        switch (mode) {
            case DistributeMode::FlexStart: break;
            case DistributeMode::FlexEnd:
                a.initialOffset = remainingSpace; break;
            case DistributeMode::Center:
                a.initialOffset = remainingSpace / 2.0f; break;
            case DistributeMode::SpaceBetween:
                if (remainingSpace <= 0.0f) break;
                if (itemCount > 1) a.spaceBetween = remainingSpace / (itemCount - 1); break;
            case DistributeMode::SpaceAround: {
                if (remainingSpace <= 0.0f) break;
                float gap = remainingSpace / itemCount;
                a.initialOffset = gap / 2.0f;
                a.spaceBetween = gap;
                break;
            }
            case DistributeMode::SpaceEvenly: {
                if (remainingSpace <= 0.0f) break;
                float gap = remainingSpace / (itemCount + 1);
                a.initialOffset = gap;
                a.spaceBetween = gap;
                break;
            }
        }
        return a;
    }

    DistributeMode toDistributeMode(JustifyContent jc) {
        switch (jc) {
            case JustifyContent::FlexStart:    return DistributeMode::FlexStart;
            case JustifyContent::FlexEnd:      return DistributeMode::FlexEnd;
            case JustifyContent::Center:       return DistributeMode::Center;
            case JustifyContent::SpaceBetween: return DistributeMode::SpaceBetween;
            case JustifyContent::SpaceAround:  return DistributeMode::SpaceAround;
            case JustifyContent::SpaceEvenly:  return DistributeMode::SpaceEvenly;
        }
    }

    DistributeMode toDistributeMode(AlignContent ac) {
        switch (ac) {
            case AlignContent::Stretch:      return DistributeMode::FlexStart;
            case AlignContent::FlexStart:    return DistributeMode::FlexStart;
            case AlignContent::FlexEnd:      return DistributeMode::FlexEnd;
            case AlignContent::Center:       return DistributeMode::Center;
            case AlignContent::SpaceBetween: return DistributeMode::SpaceBetween;
            case AlignContent::SpaceAround:  return DistributeMode::SpaceAround;
            case AlignContent::SpaceEvenly:  return DistributeMode::SpaceEvenly;
        }
    }

    Constraints FlexResolver::prepareChildConstraints() {
        auto newChildConstraints = childConstraints;
        newChildConstraints.intrinsicSizesAxis.reset();
        newChildConstraints.inheritedProperties = parentConstraints.inheritedProperties;

        return newChildConstraints;
    }

    void FlexResolver::phaseB() {
        // FIXME: this is extremely ugly and terribly handled
        resolvedGap = std::visit(Overloaded {
            [&](float availableMain) { return node->getFlexGap().resolve(Size::px(availableMain)).value_or(0.0f); },
            [&](const auto&) { return node->getFlexGap().resolve(Size::autoSize()).value_or(0.0f); },
        }, flex.axis.mainSize(availableSize));

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto position = childAsPtr->getPosition();
            if (position == Position::Absolute || position == Position::Fixed) 
                continue;

            auto selfAlign = childAsPtr->getAlignSelf();
            auto preparedChildConstraints = prepareChildConstraints();
            Measured childMeasured = *childAsPtr->measured;

            SizeRequest childRequest {
                .position = childAsPtr->shared.position,
                .specified = {.width = childAsPtr->shared.width, .height = childAsPtr->shared.height},
                .override = preparedChildConstraints.parentOverride,
                .content = {.width = std::monostate{}, .height = std::monostate{}},
                .minimum = {.width = childAsPtr->shared.minWidth, .height = childAsPtr->shared.minHeight},
                .maximum = {
                    .width = childAsPtr->shared.maxWidth ? SizeState{*childAsPtr->shared.maxWidth} : SizeState{std::monostate{}},
                    .height = childAsPtr->shared.maxHeight ? SizeState{*childAsPtr->shared.maxHeight} : SizeState{std::monostate{}},
                },
                .available = {.width = preparedChildConstraints.availableWidth, .height = preparedChildConstraints.availableHeight},
                .top = childAsPtr->shared.top,
                .right = childAsPtr->shared.right,
                .bottom = childAsPtr->shared.bottom,
                .left = childAsPtr->shared.left,
                .paddingTop = childAsPtr->shared.paddingTop.value_or(childAsPtr->shared.padding),
                .paddingRight = childAsPtr->shared.paddingRight.value_or(childAsPtr->shared.padding),
                .paddingBottom = childAsPtr->shared.paddingBottom.value_or(childAsPtr->shared.padding),
                .paddingLeft = childAsPtr->shared.paddingLeft.value_or(childAsPtr->shared.padding),
                .borderWidth = childAsPtr->shared.borderWidth,
                .margins = childAsPtr->preLayout->resolvedMargins,
                .aspectRatio = childAsPtr->shared.aspectRatio,
                .automaticWidth = AutomaticSizing::UseContent,
                .automaticHeight = AutomaticSizing::UseContent,
                // why this ternary?
                // https://www.w3.org/TR/css-flexbox-1/#min-size-auto
                .automaticMinimumWidth = flex.axis.isRow ? AutomaticMinimum::ContentBased : AutomaticMinimum::Zero,
                .automaticMinimumHeight = flex.axis.isRow ? AutomaticMinimum::Zero : AutomaticMinimum::ContentBased,
                .intrinsicWidthRequest = flex.axis.isRow ? std::optional{IntrinsicRequest::Maximum} : std::nullopt,
                .intrinsicHeightRequest = flex.axis.isRow ? std::nullopt : std::optional{IntrinsicRequest::Maximum},
            };

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childAsPtr, {
                .availableWidth = preparedChildConstraints.availableWidth,
                .widthRequest = childRequest.intrinsicWidthRequest,
                .trackIntrinsicWidth = false,
            });

            auto debugText = tree::getText(childAsPtr);
            if (debugText) {
                std::println(
                    "[flex phase B:evaluate input] '{}' fragments={} lines={}",
                    *debugText,
                    preparedChildConstraints.inlineFormatting.lineFragments().size(),
                    preparedChildConstraints.inlineFormatting.lineBoxes().size()
                );
            }

            SizeResult childSizing = evaluateSize(tree, childAsPtr, frameInfo, preparedChildConstraints, childMeasured, childRequest);

            const float* preferredMainSize = std::get_if<float>(&flex.axis.mainSize(childSizing.size));
            const auto& intrinsicMainSizes = flex.axis.isRow ? childSizing.widthIntrinsicSizes : childSizing.heightIntrinsicSizes;
            AlignItems effectiveAlign = flex.effectiveAlign(selfAlign);
            
            float flexBaseSize = preferredMainSize ? *preferredMainSize : std::get<float>(intrinsicMainSizes->maximum);
            if (debugText) {
                std::println(
                    "[flex phase B:size result] '{}' preferred-main-alt={} intrinsic-min-alt={} intrinsic-max-alt={} flex-base={}",
                    *debugText,
                    flex.axis.mainSize(childSizing.size).index(),
                    intrinsicMainSizes->minimum.index(),
                    intrinsicMainSizes->maximum.index(),
                    flexBaseSize
                );
            }
            float minMainSize = std::get<float>(flex.axis.mainSize(childSizing.minimum));
            std::optional<float> maxMainSize;
            
            // these guys took a two fucking week lecture on haskell
            // then didnt bother implementing the rest
            // omg there is a so much better language 3 steps away from here
            if (std::holds_alternative<float>(flex.axis.mainSize(childSizing.maximum))) {
                maxMainSize = std::get<float>(flex.axis.mainSize(childSizing.maximum));
            }

            flex.addItem(
                i,
                childAsPtr,
                flexBaseSize,
                minMainSize,
                maxMainSize,
                effectiveAlign,
                flex.axis.mainSize(availableSize),
                resolvedGap
            );
        }

        if (flex.currentLine.count() > 0) {
            flex.lines.push_back(std::move(flex.currentLine));
            flex.currentLine = FlexLine{};
        }

        float totalSizeFallback = 0;
        bool resolvingMinContent = flex.axis.mainResolution(parentConstraints) == AxisResolution::MinContent;

        for (auto& line : flex.lines) {
            totalSizeFallback += resolvingMinContent
                ? line.totalMinimumWithGap(resolvedGap)
                : line.totalWithGap(resolvedGap);
        }

        Axis mainAxis = flex.axis.isRow ? Axis::Width : Axis::Height;
        if (parentConstraints.intrinsicSizesAxis == mainAxis) {
            float minContent = 0.0f;
            float maxContent = 0.0f;
            for (auto& line : flex.lines) {
                minContent = std::max(minContent, line.totalMinimumWithGap(resolvedGap));
                maxContent = std::max(maxContent, line.totalWithGap(resolvedGap));
            }

            intrinsicSizes = IntrinsicSizes{.minimum = minContent, .maximum = maxContent};
        }

        availableMain = std::visit(Overloaded {
            [](float value) { return value; },
            [&](const auto&) { return totalSizeFallback; },
        }, flex.axis.mainSize(availableSize));
        
        if (node->shared.overflow == Overflow::Scroll) {
            availableMain = std::max(availableMain, totalSizeFallback);
        }

        resolvedMainSizes = flex.resolveSizes(availableMain, resolvedGap);
    }

    FlexResolver::Bounds FlexResolver::phaseC() {
        bool traceAlbumDetails = false;
        for (const auto& child : node->children) {
            auto childText = tree::getText(child.get());
            if (childText == "ALBUM") {
                traceAlbumDetails = true;
                break;
            }
        }

        for (auto& line : flex.lines) {
            line.maxCrossSize = 0.0f;

            for (auto& item : line.items) {
                auto childNode = node->children[item.childIndex].get();
                auto preparedChildConstraints = prepareChildConstraints();
                Measured childMeasured = *childNode->measured;

                flex.axis.mainAvailable(preparedChildConstraints) = Size::px(item.usedMainSize);
                preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childNode, {
                    .availableWidth = preparedChildConstraints.availableWidth,
                    .widthRequest = std::nullopt,
                    .trackIntrinsicWidth = false,
                });

                SizeRequest childRequest {
                    .position = childNode->shared.position,
                    .specified = {.width = childNode->shared.width, .height = childNode->shared.height},
                    .override = preparedChildConstraints.parentOverride,
                    .content = {.width = std::monostate{}, .height = std::monostate{}},
                    .minimum = {.width = childNode->shared.minWidth, .height = childNode->shared.minHeight},
                    .maximum = {
                        .width = childNode->shared.maxWidth ? SizeState{*childNode->shared.maxWidth} : SizeState{std::monostate{}},
                        .height = childNode->shared.maxHeight ? SizeState{*childNode->shared.maxHeight} : SizeState{std::monostate{}},
                    },
                    .available = {.width = preparedChildConstraints.availableWidth, .height = preparedChildConstraints.availableHeight},
                    .top = childNode->shared.top,
                    .right = childNode->shared.right,
                    .bottom = childNode->shared.bottom,
                    .left = childNode->shared.left,
                    .paddingTop = childNode->shared.paddingTop.value_or(childNode->shared.padding),
                    .paddingRight = childNode->shared.paddingRight.value_or(childNode->shared.padding),
                    .paddingBottom = childNode->shared.paddingBottom.value_or(childNode->shared.padding),
                    .paddingLeft = childNode->shared.paddingLeft.value_or(childNode->shared.padding),
                    .borderWidth = childNode->shared.borderWidth,
                    .margins = childNode->preLayout->resolvedMargins,
                    .aspectRatio = childNode->shared.aspectRatio,
                    .automaticWidth = AutomaticSizing::UseContent,
                    .automaticHeight = AutomaticSizing::UseContent,
                    .automaticMinimumWidth = AutomaticMinimum::Zero,
                    .automaticMinimumHeight = AutomaticMinimum::Zero,
                };

                if (flex.axis.isRow) {
                    childRequest.automaticHeight = AutomaticSizing::UseContent;
                } else {
                    childRequest.automaticWidth = AutomaticSizing::UseContent;
                }
                flex.axis.mainSize(childRequest.override) = item.usedMainSize;

                LayoutOutput childOutput = tree.layoutRecursive(childNode, frameInfo, preparedChildConstraints, childMeasured, false, std::move(childRequest));
                const SizeResult& sizeResult = childOutput.sizeResult;

                const float* preferredCrossSize = std::get_if<float>(&flex.axis.crossSize(sizeResult.size));
                item.hypotheticalCrossSize = preferredCrossSize ? *preferredCrossSize : flex.axis.crossSize(childOutput.layout);

                line.maxCrossSize = std::max(line.maxCrossSize, item.hypotheticalCrossSize);
            }
        }

        float contentCrossSize = 0;

        for (auto& line : flex.lines) 
            contentCrossSize += line.maxCrossSize;
        if (flex.lines.size() > 1) 
            contentCrossSize += resolvedGap * (flex.lines.size() - 1);
        float availableCross = contentCrossSize;

        auto placements = flex.computePlacements(
            resolvedMainSizes,
            availableMain,
            availableCross,
            resolvedGap
        );

        for (auto& p : placements) {
            size_t i = p.childIndex;
            auto childNode = node->children[i].get();
            Measured childMeasured = *childNode->measured;

            auto preparedChildConstraints = prepareChildConstraints();

            auto childPosition = flex.axis.toPhysical(
                p.mainOffset,
                p.crossOffset
            );

            preparedChildConstraints.origin = childPosition;
            preparedChildConstraints.cursor = childPosition;
            flex.axis.mainAvailable(preparedChildConstraints) = Size::px(p.mainSize);
            flex.axis.crossAvailable(preparedChildConstraints) = Size::px(p.lineCrossSize);
            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childNode, {
                .availableWidth = preparedChildConstraints.availableWidth,
                .widthRequest = std::nullopt,
                .trackIntrinsicWidth = false,
            });

            SizeRequest childRequest {
                .position = childNode->shared.position,
                .specified = {.width = childNode->shared.width, .height = childNode->shared.height},
                .override = preparedChildConstraints.parentOverride,
                .content = {.width = std::monostate{}, .height = std::monostate{}},
                .minimum = {.width = childNode->shared.minWidth, .height = childNode->shared.minHeight},
                .maximum = {
                    .width = childNode->shared.maxWidth ? SizeState{*childNode->shared.maxWidth} : SizeState{std::monostate{}},
                    .height = childNode->shared.maxHeight ? SizeState{*childNode->shared.maxHeight} : SizeState{std::monostate{}},
                },
                .available = {.width = preparedChildConstraints.availableWidth, .height = preparedChildConstraints.availableHeight},
                .top = childNode->shared.top,
                .right = childNode->shared.right,
                .bottom = childNode->shared.bottom,
                .left = childNode->shared.left,
                .paddingTop = childNode->shared.paddingTop.value_or(childNode->shared.padding),
                .paddingRight = childNode->shared.paddingRight.value_or(childNode->shared.padding),
                .paddingBottom = childNode->shared.paddingBottom.value_or(childNode->shared.padding),
                .paddingLeft = childNode->shared.paddingLeft.value_or(childNode->shared.padding),
                .borderWidth = childNode->shared.borderWidth,
                .margins = childNode->preLayout->resolvedMargins,
                .aspectRatio = childNode->shared.aspectRatio,
                .automaticWidth = AutomaticSizing::UseContent,
                .automaticHeight = AutomaticSizing::UseContent,
                .automaticMinimumWidth = AutomaticMinimum::Zero,
                .automaticMinimumHeight = AutomaticMinimum::Zero,
            };

            flex.axis.mainSize(childRequest.override) = p.mainSize;
            if (flex.axis.isRow) {
                childRequest.automaticHeight = p.alignment == AlignItems::Stretch ? AutomaticSizing::UseAvailable : AutomaticSizing::UseContent;
            } else {
                childRequest.automaticWidth = p.alignment == AlignItems::Stretch ? AutomaticSizing::UseAvailable : AutomaticSizing::UseContent;
            }

            auto debugText = tree::getText(childNode);
            if (debugText) {
                std::println(
                    "[flex phase C:input] '{}' main-offset={} main-size={} fragments={} lines={}",
                    *debugText,
                    p.mainOffset,
                    p.mainSize,
                    preparedChildConstraints.inlineFormatting.lineFragments().size(),
                    preparedChildConstraints.inlineFormatting.lineBoxes().size()
                );
            }

            LayoutOutput childOutput = tree.layoutRecursive(childNode, frameInfo, preparedChildConstraints, childMeasured, mutate, std::move(childRequest));
            const auto& childLayout = childOutput.layout;

            if (mutate && traceAlbumDetails) {
                std::println(
                    "[album details] parent={} child={} row={} index={} authored={}({})x{}({}) placement=({}, {}) allocated={}x{} box=({}, {}) {}x{}",
                    node->id,
                    childNode->id,
                    flex.axis.isRow,
                    i,
                    childNode->shared.width.value,
                    static_cast<int>(childNode->shared.width.unit),
                    childNode->shared.height.value,
                    static_cast<int>(childNode->shared.height.unit),
                    p.mainOffset,
                    p.crossOffset,
                    p.mainSize,
                    p.lineCrossSize,
                    childLayout.computedBox.x,
                    childLayout.computedBox.y,
                    childLayout.computedBox.width,
                    childLayout.computedBox.height
                );
            }

            if (debugText) {
                std::println(
                    "[flex phase C:output] '{}' width={} height={} consumed-height={} main-extent={}",
                    *debugText,
                    childLayout.computedBox.width,
                    childLayout.computedBox.height,
                    childLayout.consumedHeight,
                    flex.axis.mainSize(childLayout)
                );
            }

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        return {maxX, maxY};
    }

}
