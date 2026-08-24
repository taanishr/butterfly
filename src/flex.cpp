#include "flex.hpp"
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

    Constraints FlexResolver::prepareChildConstraints(TreeNode* child) {
        auto newChildConstraints = childConstraints;
        newChildConstraints.intrinsicSizesAxis.reset();

        newChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            {
                .availableWidth = childAvailableWidth,
                .widthRequest = newChildConstraints.widthResolution ? std::optional{*newChildConstraints.widthResolution == AxisResolution::MinContent ? IntrinsicRequest::Minimum : IntrinsicRequest::Maximum} : std::nullopt,
                .trackIntrinsicWidth = false,
            }
        );
        newChildConstraints.availableWidth = childAvailableWidth;
        newChildConstraints.inheritedProperties = parentConstraints.inheritedProperties;

        return newChildConstraints;
    }

    float FlexResolver::determineAvailableMain(float contentMainSize)
    {
        const auto& mainSize = flex.axis.mainExplicit(measured);

        float availableMain;
        if (mainSize) {
            availableMain = parentAvailableMain().isAuto() ? contentMainSize : parentAvailableMain().value;
        } else {
            switch (mainSize.error()) {
                case SizeError::Auto:
                    availableMain =
                        flex.axis.isRow &&
                        std::holds_alternative<std::monostate>(flex.axis.mainOverride(parentConstraints)) &&
                        !parentConstraints.shrinkWidthToFit
                            ? (parentAvailableMain().isAuto() ? contentMainSize : parentAvailableMain().value)
                            : contentMainSize;
                    break;
                case SizeError::IndefiniteBasis:
                case SizeError::ContentDependent:
                    availableMain = contentMainSize;
                    break;
                case SizeError::FractionRequiresContext:
                    // fr does not make sense as a flex container main size.
                    availableMain = contentMainSize;
                    break;
            }
        }

        if (node->shared.overflow == Overflow::Scroll) {
            availableMain = std::max(availableMain, contentMainSize);
        }

        return availableMain;
    }

    float FlexResolver::determineAvailableCross(
        float contentCrossSize
    ) {
        auto& crossSize = flex.axis.crossExplicit(measured);

        if (crossSize) {
            return parentAvailableCross().isAuto() ? contentCrossSize : parentAvailableCross().value;
        }

        switch (crossSize.error()) {
            case SizeError::Auto:
                if (!flex.axis.isRow &&
                    std::holds_alternative<std::monostate>(flex.axis.crossOverride(parentConstraints)) &&
                    !parentConstraints.shrinkWidthToFit) {
                    return parentAvailableCross().isAuto() ? contentCrossSize : parentAvailableCross().value;
                }
                return contentCrossSize;
            case SizeError::IndefiniteBasis:
            case SizeError::ContentDependent:
                return contentCrossSize;
            case SizeError::FractionRequiresContext:
                // fr does not make sense as a flex container cross size.
                return contentCrossSize;
        }
    }

    void FlexResolver::phaseB() {
        resolvedGap = node->getFlexGap()
            .resolve(parentAvailableMain())
            .value_or(0.0f);

        for (uint64_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto position = childAsPtr->getPosition();
            if (position == Position::Absolute || position == Position::Fixed) 
                continue;

            auto selfAlign = childAsPtr->getAlignSelf();
            auto preparedChildConstraints = prepareChildConstraints(childAsPtr);
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
                .automaticWidth = preparedChildConstraints.shrinkWidthToFit ? AutomaticSizing::UseContent : AutomaticSizing::UseAvailable,
                .automaticHeight = AutomaticSizing::UseContent,
                .automaticMinimumWidth = AutomaticMinimum::Zero,
                .automaticMinimumHeight = AutomaticMinimum::Zero,
                .intrinsicWidthRequest = flex.axis.isRow ? std::optional{IntrinsicRequest::Maximum} : std::nullopt,
                .intrinsicHeightRequest = flex.axis.isRow ? std::nullopt : std::optional{IntrinsicRequest::Maximum},
            };

            SizeResult childSizing = evaluateSize(tree, childAsPtr, frameInfo, preparedChildConstraints, childMeasured, childRequest);

            const float* preferredMainSize = std::get_if<float>(&flex.axis.mainSize(childSizing.size));
            const auto& intrinsicMainSizes = flex.axis.isRow ? childSizing.widthIntrinsicSizes : childSizing.heightIntrinsicSizes;
            AlignItems effectiveAlign = flex.effectiveAlign(selfAlign);
            
            float flexBaseSize = preferredMainSize ? *preferredMainSize : std::get<float>(intrinsicMainSizes->maximum);
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
                parentAvailableMain(),
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

        availableMain = determineAvailableMain(totalSizeFallback);
        resolvedMainSizes = flex.resolveSizes(availableMain, resolvedGap);
    }

    FlexResolver::Bounds FlexResolver::phaseC() {
        Axis crossAxis = flex.axis.isRow ? Axis::Height : Axis::Width;
        bool parentRequestsIntrinsicCross = parentConstraints.intrinsicSizesAxis == crossAxis;

        for (auto& line : flex.lines) {
            line.maxCrossSize = 0.0f;
            line.intrinsicMinCrossSize = 0.0f;
            line.intrinsicMaxCrossSize = 0.0f;

            for (auto& item : line.items) {
                auto childNode = node->children[item.childIndex].get();
                const auto& crossRequest = flex.axis.crossSize(childNode->shared);
                const auto& minCrossRequest = flex.axis.minCrossSize(childNode->shared);
                const auto& maxCrossRequest = flex.axis.maxCrossSize(childNode->shared);
                auto resolvedCrossSize = resolveCrossSize(childNode);
                auto preparedChildConstraints = prepareChildConstraints(childNode);
                Measured childMeasured = *childNode->measured;
                bool needsIntrinsicCross = parentRequestsIntrinsicCross || crossRequest.isContentDependent() || minCrossRequest.isContentDependent() || (maxCrossRequest.has_value() && maxCrossRequest->isContentDependent());

                flex.axis.mainAvailable(preparedChildConstraints) = Size::px(item.usedMainSize);
                // preparedChildConstraints.parentOverride = item.usedMainSize;
                flex.axis.mainOverride(preparedChildConstraints) = item.usedMainSize;
                flex.axis.mainExplicit(childMeasured) = item.usedMainSize;
                if (childNode->shared.aspectRatio)
                    transferAspectRatio(
                        childMeasured.explicitWidth,
                        childMeasured.explicitHeight,
                        *childNode->shared.aspectRatio
                    );
                    
                if (needsIntrinsicCross) preparedChildConstraints.intrinsicSizesAxis = crossAxis;
                preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childNode, {
                    .availableWidth = preparedChildConstraints.availableWidth,
                    .widthRequest = preparedChildConstraints.widthResolution ? std::optional{*preparedChildConstraints.widthResolution == AxisResolution::MinContent ? IntrinsicRequest::Minimum : IntrinsicRequest::Maximum} : std::nullopt,
                    .trackIntrinsicWidth = preparedChildConstraints.intrinsicSizesAxis == Axis::Width,
                });

                const auto& childOutput = tree.speculateLayout(
                    frameInfo,
                    childNode,
                    preparedChildConstraints,
                    childMeasured
                );

                if (crossRequest.isContentDependent()) 
                    resolvedCrossSize = resolveIntrinsicSize(crossRequest, *childOutput.intrinsicSizes, parentAvailableCross());

                item.minCrossSize = minCrossRequest.isContentDependent() ? resolveIntrinsicSize(minCrossRequest, *childOutput.intrinsicSizes, parentAvailableCross()) : minCrossRequest.resolveOr(parentAvailableCross(), 0.0f);
                if (maxCrossRequest.has_value() && maxCrossRequest->isContentDependent()) {
                    item.maxCrossSize = resolveIntrinsicSize(*maxCrossRequest, *childOutput.intrinsicSizes, parentAvailableCross());
                } else if (maxCrossRequest.has_value()) {
                    auto resolvedMaxCross = maxCrossRequest->resolve(parentAvailableCross());
                    if (resolvedMaxCross) 
                        item.maxCrossSize = *resolvedMaxCross;
                }

                item.hypotheticalCrossSize = resolvedCrossSize ? *resolvedCrossSize : flex.axis.crossSize(childOutput.layout);
                item.hypotheticalCrossSize = std::max(item.hypotheticalCrossSize, item.minCrossSize);
                if (item.maxCrossSize.has_value()) 
                    item.hypotheticalCrossSize = std::min(item.hypotheticalCrossSize, *item.maxCrossSize);

                line.maxCrossSize = std::max(line.maxCrossSize, item.hypotheticalCrossSize);
                if (parentRequestsIntrinsicCross && childOutput.intrinsicSizes.has_value()) {
                    float childMinCross = childOutput.intrinsicSizes->minimum;
                    float childMaxCross = childOutput.intrinsicSizes->maximum;
                    childMinCross = std::max(childMinCross, item.minCrossSize);
                    childMaxCross = std::max(childMaxCross, item.minCrossSize);
                    if (item.maxCrossSize.has_value()) {
                        childMinCross = std::min(childMinCross, *item.maxCrossSize);
                        childMaxCross = std::min(childMaxCross, *item.maxCrossSize);
                    }
                    line.intrinsicMinCrossSize = std::max(line.intrinsicMinCrossSize, childMinCross);
                    line.intrinsicMaxCrossSize = std::max(line.intrinsicMaxCrossSize, childMaxCross);
                }
            }
        }

        float naturalCross = 0;

        for (auto& line : flex.lines) 
            naturalCross += line.maxCrossSize;
        if (flex.lines.size() > 1) 
            naturalCross += resolvedGap * (flex.lines.size() - 1);
        
        if (parentRequestsIntrinsicCross) {
            float minContent = 0.0f;
            float maxContent = 0.0f;
            for (auto& line : flex.lines) {
                minContent += line.intrinsicMinCrossSize;
                maxContent += line.intrinsicMaxCrossSize;
            }
            if (flex.lines.size() > 1) {
                float totalGap = resolvedGap * (flex.lines.size() - 1);
                minContent += totalGap;
                maxContent += totalGap;
            }
            intrinsicSizes = IntrinsicSizes{.minimum = minContent, .maximum = maxContent};
        }
        float availableCross = determineAvailableCross(naturalCross);

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

            auto preparedChildConstraints = prepareChildConstraints(childNode);

            auto childPosition = flex.axis.toPhysical(
                p.mainOffset,
                p.crossOffset
            );

            preparedChildConstraints.origin = childPosition;
            preparedChildConstraints.cursor = childPosition;
            flex.axis.mainAvailable(preparedChildConstraints) = Size::px(p.mainSize);
            flex.axis.mainOverride(preparedChildConstraints) = p.mainSize;

            flex.axis.mainExplicit(childMeasured) = p.mainSize;

            float finalCrossSize = p.crossSize;
            if (p.crossSizeOverride.has_value()) finalCrossSize = *p.crossSizeOverride;

            flex.axis.crossAvailable(preparedChildConstraints) =
                Size::px(finalCrossSize);
            // preparedChildConstraints.parentOverride = finalCrossSize;
            flex.axis.crossOverride(preparedChildConstraints) = finalCrossSize;
            flex.axis.crossExplicit(childMeasured) = finalCrossSize;
            flex.axis.crossShrinkToFit(preparedChildConstraints) =
                p.needsCrossShrinkToFit;

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
                childNode,
                {
                    .availableWidth = preparedChildConstraints.availableWidth,
                    .widthRequest = preparedChildConstraints.widthResolution ? std::optional{*preparedChildConstraints.widthResolution == AxisResolution::MinContent ? IntrinsicRequest::Minimum : IntrinsicRequest::Maximum} : std::nullopt,
                    .trackIntrinsicWidth = false,
                }
            );

            std::optional<LayoutOutput> finalChildOutput;
            const LayoutOutput* childOutput;
            if (mutate) {
                finalChildOutput = tree.layoutPhase(
                    childNode,
                    frameInfo,
                    preparedChildConstraints,
                    childMeasured
                );
                childOutput = &*finalChildOutput;
            } else {
                childOutput = &tree.speculateLayout(
                    frameInfo,
                    childNode,
                    preparedChildConstraints,
                    childMeasured
                );
            }
            const auto& childLayout = childOutput->layout;

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        return {maxX, maxY};
    }

}
