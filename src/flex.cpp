#include "flex.hpp"
#include "new_arch.hpp"
#include "new_sizing.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "render_tree.hpp"
#include <algorithm>
#include <format>
#include <optional>
#include <print>
#include <string>
#include <variant>

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
        newChildConstraints.inheritedProperties = parentConstraints.inheritedProperties;

        return newChildConstraints;
    }

    void FlexResolver::phaseB() {
        // FIXME: this is extremely ugly and terribly handled
        resolvedGap = std::visit(Overloaded {
            [&](float availableMain) { return node->getFlexGap().resolve(Size::px(availableMain)).value_or(0.0f); },
            [&](const auto&) { return node->getFlexGap().resolve(Size::autoSize()).value_or(0.0f); },
        }, flex.axis.mainSize(availableSize));

       // if (node->id == 106)
       //     std::println("initial w: {}", node->measured->explicitWidth.value_or(-1.0f));

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
                .override = {.width = std::monostate{}, .height = std::monostate{}},
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
                /*
                    1) only the main axis is content based
                    2) if overflow: scroll is enabled, the main axis is zero-based (4.5)
                */
                .automaticMinimumWidth = flex.axis.isRow
                                        ? (childAsPtr->shared.overflow == style::Overflow::Scroll
                                            ? AutomaticMinimum::Zero
                                            : AutomaticMinimum::ContentBased
                                          )
                                        : AutomaticMinimum::Zero,
                .automaticMinimumHeight = flex.axis.isRow
                                        ? AutomaticMinimum::Zero
                                        : (childAsPtr->shared.overflow == style::Overflow::Scroll
                                            ? AutomaticMinimum::Zero
                                            : AutomaticMinimum::ContentBased
                                          ),
                .intrinsicWidthRequest = flex.axis.isRow ? std::optional{IntrinsicRequest::Both} : std::nullopt,
                .intrinsicHeightRequest = flex.axis.isRow ? std::nullopt : std::optional{IntrinsicRequest::Both},

                .tag = "flex phase B, main size"
            };

            // overrides
            if (intrinsicWidthRequest) {
                childRequest.intrinsicWidthRequest = intrinsicWidthRequest;
            }

            if (intrinsicHeightRequest) {
                childRequest.intrinsicHeightRequest = intrinsicHeightRequest;
            }


            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childAsPtr, {
                .availableWidth = preparedChildConstraints.availableWidth,
                .widthRequest = childRequest.intrinsicWidthRequest,
                .trackIntrinsicWidth = false,
            });

            SizeResult childSizing = evaluateSize(tree, childAsPtr, frameInfo, preparedChildConstraints, childMeasured, childRequest, sizeCache);

            const SizeState& preferredMainSize = flex.axis.mainSize(childSizing.outerSize);
            const auto& measuredMainIntrinsicSizes = flex.axis.isRow ? childSizing.widthIntrinsicSizes : childSizing.heightIntrinsicSizes;
            IntrinsicResult mainIntrinsicSizes {
                .minimum = measuredMainIntrinsicSizes ? measuredMainIntrinsicSizes->minimum : preferredMainSize,
                .maximum = measuredMainIntrinsicSizes ? measuredMainIntrinsicSizes->maximum : preferredMainSize,
            };

            AlignItems effectiveAlign = flex.effectiveAlign(selfAlign);

            const SizeState& flexBaseSize = std::holds_alternative<float>(preferredMainSize) ? preferredMainSize : mainIntrinsicSizes.maximum;
            const SizeState& minimumMainSize = flex.axis.mainSize(childSizing.minimum);
            const SizeState& maximumMainSize = flex.axis.mainSize(childSizing.maximum);

            if (childAsPtr->id == 104) {
                std::println(" minimum main size: {}, pref main size: {} intrinsic main min {}, intrinsic main max {}, flex base size: {}", describeSizeState(minimumMainSize), describeSizeState(preferredMainSize), describeSizeState(mainIntrinsicSizes.minimum), describeSizeState(mainIntrinsicSizes.maximum), describeSizeState(flexBaseSize));
            }

            flex.addItem(
                i,
                childAsPtr,
                flexBaseSize,
                minimumMainSize,
                maximumMainSize,
                mainIntrinsicSizes,
                effectiveAlign,
                flex.axis.mainSize(availableSize),
                resolvedGap
            );
        }

        if (flex.currentLine.count() > 0) {
            flex.lines.push_back(flex.currentLine);
            flex.currentLine = FlexLine{};
        }

        float totalFlexBaseSize = 0;
        for (auto& line : flex.lines) {
            totalFlexBaseSize += line.totalWithGap(resolvedGap);
        }
        
        availableMain = std::visit(Overloaded {
            [](float value) { return value; },
            [&](const auto&) { return totalFlexBaseSize; },
        }, flex.axis.mainSize(availableSize));
        
        if (node->shared.overflow == Overflow::Scroll) {
            availableMain = std::max(availableMain, totalFlexBaseSize);
        }


        resolvedMainSizes = flex.resolveSizes(availableMain, resolvedGap);
    }

    FlexResolver::FlexResult FlexResolver::phaseC() {
        float minimumCrossContribution = 0.0f;
        float maximumCrossContribution = 0.0f;

        for (auto& line : flex.lines) {
            line.maxCrossSize = 0.0f;
            float lineMinimumCrossContribution = 0.0f;
            float lineMaximumCrossContribution = 0.0f;

            const bool containsNode104 = std::ranges::any_of(
                line.items,
                [](const FlexItem& item) { return item.childId == 104; }
            );

            // if (containsNode104) {
            //     lineMinimumCrossContribution = 0.0f;
            //     lineMaximumCrossContribution = 100.0f;
            //     line.maxCrossSize = 100.0f;
            // }else {
            for (auto& item : line.items) {
                auto childNode = node->children[item.childIndex].get();
                auto preparedChildConstraints = prepareChildConstraints();
                Measured childMeasured = *childNode->measured;

                SizePair childAvailableSize {
                    .width = preparedChildConstraints.availableWidth,
                    .height = preparedChildConstraints.availableHeight,
                };
                flex.axis.mainSize(childAvailableSize) = item.usedMainSize;
    
                SizeRequest childRequest {
                    .position = childNode->shared.position,
                    .specified = {.width = childNode->shared.width, .height = childNode->shared.height},
                    .override = {.width = std::monostate{}, .height = std::monostate{}},
                    .content = {.width = std::monostate{}, .height = std::monostate{}},
                    .minimum = {.width = childNode->shared.minWidth, .height = childNode->shared.minHeight},
                    .maximum = {
                        .width = childNode->shared.maxWidth ? SizeState{*childNode->shared.maxWidth} : SizeState{std::monostate{}},
                        .height = childNode->shared.maxHeight ? SizeState{*childNode->shared.maxHeight} : SizeState{std::monostate{}},
                    },
                    .available = childAvailableSize,
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
                    .intrinsicWidthRequest = flex.axis.isRow ? std::nullopt : std::optional{IntrinsicRequest::Both},
                    .intrinsicHeightRequest = flex.axis.isRow ? std::optional{IntrinsicRequest::Both} : std::nullopt,
                    .tag = "flex phase C, cross Size req"
                };


                // overrides for intrinsic reqs
                if (intrinsicWidthRequest) {
                    if (containsNode104) {
                        std::println("overriding cross axis");
                    }
                    childRequest.intrinsicWidthRequest = intrinsicWidthRequest;
                }

                if (intrinsicHeightRequest) {
                    childRequest.intrinsicHeightRequest = intrinsicHeightRequest;
                }

                preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childNode, {
                    .availableWidth = childAvailableSize.width,
                    .widthRequest = childRequest.intrinsicWidthRequest,
                    .trackIntrinsicWidth = false,
                });


                if (flex.axis.isRow) {
                    childRequest.automaticHeight = AutomaticSizing::UseContent;
                } else {
                    childRequest.automaticWidth = AutomaticSizing::UseContent;
                }


                flex.axis.mainSize(childRequest.override) = item.usedMainSize;

                // seems to only correlate with the situation that causes the cross size to reduce
                // if (childNode->id == 104) {
                //     childRequest.override.width = 100.0f;
                // }


                LayoutOutput childOutput = tree.layoutRecursive(childNode, frameInfo, preparedChildConstraints, childMeasured, false, childRequest);
                const SizeResult& sizeResult = childOutput.sizeResult;

                const float* preferredCrossSize = std::get_if<float>(&flex.axis.crossSize(sizeResult.outerSize));
                if (childNode->id == 104) {
                    std::println("preferred crossSize: {}, fallback cross size: {}", preferredCrossSize ? *preferredCrossSize : -1.0f, flex.axis.crossSize(childOutput.layout));
                }
                item.hypotheticalCrossSize = preferredCrossSize ? *preferredCrossSize : flex.axis.crossSize(childOutput.layout);

                const auto& measuredCrossIntrinsicSizes = flex.axis.isRow ? sizeResult.heightIntrinsicSizes : sizeResult.widthIntrinsicSizes;

                // question whether this fallback is appropriate
            
                // if (childNode->id == 104) {
                //     std::println("hypothetical cross size: {}", item.hypotheticalCrossSize);
                // }

                // right now, we aren't collecting intrinsic sizes... so this turns out to be mostly useless

                // ok; so this cannot willy nilly change cross sizes, but CAN expand them
                //
                float childMinimumCrossContribution = std::get<float>(measuredCrossIntrinsicSizes->minimum);
                float childMaximumCrossContribution = std::get<float>(measuredCrossIntrinsicSizes->maximum);

                // the offending line is thus... the minimum
                if (containsNode104) {
                    std::println("childMinimumCrossContribution: {}", childMinimumCrossContribution);
                }
                // lineMinimumCrossContribution = std::max(lineMinimumCrossContribution, childMinimumCrossContribution);
                lineMaximumCrossContribution = std::max(lineMaximumCrossContribution, childMaximumCrossContribution);
                line.maxCrossSize = std::max(line.maxCrossSize, item.hypotheticalCrossSize);
            }
    



            minimumCrossContribution += lineMinimumCrossContribution;
            maximumCrossContribution += lineMaximumCrossContribution;
        }

        float contentCrossSize = 0;

        for (auto& line : flex.lines) 
            contentCrossSize += line.maxCrossSize;
        if (flex.lines.size() > 1) 
            contentCrossSize += resolvedGap * (flex.lines.size() - 1);

        if (flex.lines.size() > 1) {
            float crossGap = resolvedGap * (flex.lines.size() - 1);
            minimumCrossContribution += crossGap;
            maximumCrossContribution += crossGap;
        }

        float availableCross = std::visit(Overloaded {
            [](float value) { return value; },
            [&](const auto&) { 
                // if (containsNode104) {
                //     return 100.0f;
                // }else {
                //     return contentCrossSize;
                // }

                return contentCrossSize;
             },
        }, flex.axis.crossSize(availableSize));

        auto placements = flex.computePlacements(
            resolvedMainSizes,
            availableMain,
            availableCross,
            resolvedGap
        );

        for (auto& placement : placements) {
            size_t i = placement.childIndex;
            auto childNode = node->children[i].get();
            Measured childMeasured = *childNode->measured;

            auto preparedChildConstraints = prepareChildConstraints();

            auto childPosition = flex.axis.toPhysical(
                placement.mainOffset,
                placement.crossOffset
            );

            preparedChildConstraints.origin = childPosition;
            preparedChildConstraints.cursor = childPosition;
            SizePair childAvailableSize {
                .width = preparedChildConstraints.availableWidth,
                .height = preparedChildConstraints.availableHeight,
            };

            flex.axis.mainSize(childAvailableSize) = placement.mainSize;
            flex.axis.crossSize(childAvailableSize) = placement.lineCrossSize;
      
            // also wrong, but overriding this doesnt fix anything...?
            //if (node->id == 106)
            //    std::println("cross size: {}", placement.lineCrossSize);
  
            //if (node->id == 106) {
            //    float maxLineboxSize = 0.0f;
            //    for (auto& linebox : preparedChildConstraints.inlineFormatting.lineBoxes()) {
            //        maxLineboxSize = std::max(maxLineboxSize, linebox.width);
            //    }
            //    std::println("max linebox width: {}", maxLineboxSize);
            //}

            // is the cross size request being encoded properly? why is the cross size being recording as 676
            SizeRequest childRequest {
                .position = childNode->shared.position,
                .specified = {.width = childNode->shared.width, .height = childNode->shared.height},
                .override = {.width = std::monostate{}, .height = std::monostate{}},
                .content = {.width = std::monostate{}, .height = std::monostate{}},
                .minimum = {.width = childNode->shared.minWidth, .height = childNode->shared.minHeight},
                .maximum = {
                    .width = childNode->shared.maxWidth ? SizeState{*childNode->shared.maxWidth} : SizeState{std::monostate{}},
                    .height = childNode->shared.maxHeight ? SizeState{*childNode->shared.maxHeight} : SizeState{std::monostate{}},
                },
                .available = childAvailableSize,
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

                .tag = "flex phase C, final req"
            };


            // overrides for intrinsic width/h
            if (intrinsicWidthRequest) {
                childRequest.intrinsicWidthRequest = intrinsicWidthRequest;
            }

            if (intrinsicHeightRequest) {
                childRequest.intrinsicHeightRequest = intrinsicHeightRequest;
            }

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childNode, {
                .availableWidth = childAvailableSize.width,
                .widthRequest = childRequest.intrinsicWidthRequest,
                .trackIntrinsicWidth = false,
            });


            // // this is the actual change site where you can alter things
            // if (childNode->id == 104) {
            //     childRequest.available.width = 100.0f;
            // }


            // this main size is getting miscomputed... somehow
            flex.axis.mainSize(childRequest.override) = placement.mainSize;
            // why does an override not impact the actual size?

            if (flex.axis.isRow) {
                childRequest.automaticHeight = placement.alignment == AlignItems::Stretch ? AutomaticSizing::UseAvailable : AutomaticSizing::UseContent;
            } else {
                childRequest.automaticWidth = placement.alignment == AlignItems::Stretch ? AutomaticSizing::UseAvailable : AutomaticSizing::UseContent;
            }



            // if (childNode->id == 106) {
            //     // childRequest.specified.width = 10.0f;
            //     // childRequest.available.width = 10.0f;
            //     if (std::holds_alternative<float>(childRequest.available.width)) {
            //         std::println("main size: {}", placement.mainSize);
            //         std::println("cross size: {}", placement.lineCrossSize);
            //         std::println("avail width: {}", *std::get_if<float>(&childRequest.available.width));
            //     }
            // }
            
            LayoutOutput childOutput = tree.layoutRecursive(childNode, frameInfo, preparedChildConstraints, childMeasured, mutate, childRequest);
            const auto& childLayout = childOutput.layout;

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.consumedHeight);
        }

        float minimumMainContribution = 0.0f;
        float maximumMainContribution = 0.0f;

        for (auto& line : flex.lines) {
            minimumMainContribution = std::max(minimumMainContribution, line.totalMinimumContributionWithGap(resolvedGap));
            maximumMainContribution = std::max(maximumMainContribution, line.totalMaximumContributionWithGap(resolvedGap));
        }


        return {
            .bounds = {.maxX = maxX, .maxY = maxY},
            .mainIntrinsicSizes = {.minimum = minimumMainContribution, .maximum = maximumMainContribution},
            .crossIntrinsicSizes = {.minimum = minimumCrossContribution, .maximum = maximumCrossContribution},
        };
    }

}
