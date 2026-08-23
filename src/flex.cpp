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

    IntrinsicSizes layoutIntrinsicMain(RenderTree& tree, TreeNode* child, const FrameInfo& frameInfo, Constraints constraints, Measured measured, AxisHelper& axis) {
        axis.mainShrinkToFit(constraints) = true;
        axis.mainResolution(constraints) = AxisResolution::MaxContent;
        axis.mainExplicit(measured) = std::unexpected(SizeError::Auto);
        constraints.intrinsicSizesAxis = axis.isRow ? Axis::Width : Axis::Height;

        constraints.inlineFormatting = buildIsolatedInlineBoxes(child, {
            .availableWidth = constraints.availableWidth,
            .widthRequest = constraints.widthResolution ? std::optional{*constraints.widthResolution == AxisResolution::MinContent ? IntrinsicRequest::Minimum : IntrinsicRequest::Maximum} : std::nullopt,
            .trackIntrinsicWidth = constraints.intrinsicSizesAxis == Axis::Width,
        });

        const auto& output = tree.speculateLayout(
            frameInfo,
            child,
            constraints,
            measured
        );
        if (output.intrinsicSizes.has_value()) {
            return *output.intrinsicSizes;
        }

        float size = axis.mainSize(output.layout);
        return {
            .minimum = size,
            .maximum = size,
        };
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

    float FlexResolver::determineFlexBaseSize(std::expected<float, SizeError>& mainSize, const std::optional<IntrinsicSizes>& intrinsicSizes) {
        if (mainSize) return *mainSize;
        return intrinsicSizes->maximum;
    }

    float FlexResolver::determineMinMainSize(TreeNode* child, std::expected<float, SizeError>& mainSize, const std::optional<IntrinsicSizes>& intrinsicSizes) {
        const auto& request = flex.axis.minMainSize(child->shared);
        if (request.isContentDependent()) 
            return resolveIntrinsicSize(request, *intrinsicSizes, parentAvailableMain());

        auto resolvedMinMain = resolveMainSize(request);
        if (resolvedMinMain) 
            return *resolvedMinMain;

        if (resolvedMinMain.error() ==
            SizeError::FractionRequiresContext) {
            // fr does not make sense as a flex min-size.
            return 0.0f;
        }

        if (child->shared.overflow != Overflow::Visible) return 0.0f;

        float minMainSize = intrinsicSizes->minimum;
        if (mainSize) {
            minMainSize = std::min(minMainSize, *mainSize);
        }
        return minMainSize;
    }

    std::optional<float> FlexResolver::determineMaxMainSize(TreeNode* child, const std::optional<IntrinsicSizes>& intrinsicSizes) {
        const auto& request = flex.axis.maxMainSize(child->shared);
        if (!request.has_value()) return std::nullopt;
        if (request->isContentDependent()) return resolveIntrinsicSize(*request, *intrinsicSizes, parentAvailableMain());

        auto resolved = resolveMainSize(*request);
        if (!resolved) return std::nullopt;
        return *resolved;
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
            auto mainSize = resolveMainSize(flex.axis.mainSize(childAsPtr->shared));
            auto resolvedCrossSize = resolveCrossSize(childAsPtr);
            AlignItems effectiveAlign = flex.effectiveAlign(selfAlign);

            auto preparedChildConstraints = prepareChildConstraints(childAsPtr);

            Measured childMeasured = *childAsPtr->measured;

            const auto& mainRequest = flex.axis.mainSize(childAsPtr->shared);
            const auto& minMainRequest = flex.axis.minMainSize(childAsPtr->shared);
            const auto& maxMainRequest = flex.axis.maxMainSize(childAsPtr->shared);
            auto resolvedMinMain = resolveMainSize(minMainRequest);
            bool needsIntrinsicMinimum = !resolvedMinMain && resolvedMinMain.error() != SizeError::FractionRequiresContext && childAsPtr->shared.overflow == Overflow::Visible;

            if (childAsPtr->shared.aspectRatio) {
                if (flex.axis.isRow) {
                    transferAspectRatio(mainSize, resolvedCrossSize, *childAsPtr->shared.aspectRatio);
                } else {
                    transferAspectRatio(resolvedCrossSize, mainSize, *childAsPtr->shared.aspectRatio);
                }
            }

            bool needsIntrinsicMain = !mainSize || needsIntrinsicMinimum || minMainRequest.isContentDependent() || (maxMainRequest.has_value() && maxMainRequest->isContentDependent());
            std::optional<IntrinsicSizes> intrinsicMainSizes;
            if (needsIntrinsicMain) intrinsicMainSizes = layoutIntrinsicMain(tree, childAsPtr, frameInfo, preparedChildConstraints, childMeasured, flex.axis);
            if (mainRequest.isContentDependent()) mainSize = resolveIntrinsicSize(mainRequest, *intrinsicMainSizes, parentAvailableMain());

            float flexBaseSize = determineFlexBaseSize(mainSize, intrinsicMainSizes);
            float minMainSize = determineMinMainSize(childAsPtr, mainSize, intrinsicMainSizes);
            auto maxMainSize = determineMaxMainSize(childAsPtr, intrinsicMainSizes);

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
