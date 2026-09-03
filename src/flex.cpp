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
                .available = availableSize,
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
                .minimum = measuredMainIntrinsicSizes->minimum,
                .maximum = measuredMainIntrinsicSizes->maximum
            };

            AlignItems effectiveAlign = flex.effectiveAlign(selfAlign);

            const SizeState& flexBaseSize = std::holds_alternative<float>(preferredMainSize) ? preferredMainSize : mainIntrinsicSizes.maximum;
            const SizeState& minimumMainSize = flex.axis.mainSize(childSizing.minimum);
            const SizeState& maximumMainSize = flex.axis.mainSize(childSizing.maximum);

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

        if (!flex.axis.isRow && mutate) {
            bool containsBodyFlex = false;
            for (const auto& line : flex.lines) {
                for (const auto& item : line.items) {
                    if (item.childId == 21) {
                        containsBodyFlex = true;
                        break;
                    }
                }
            }

            if (containsBodyFlex) {
                std::println(
                    "[Root height allocation] container={} available-height={} total-base-height={} resolved-total-height={} intrinsic-height-request={}",
                    node->id,
                    describeSizeState(availableSize.height),
                    totalFlexBaseSize,
                    resolvedMainSizes.overallTotalAfter,
                    intrinsicHeightRequest.has_value()
                );

                for (const auto& line : flex.lines) {
                    for (const auto& item : line.items) {
                        bool isBodyFlex = item.childId == 21;

                        std::println(
                            "[Root height item] child={} body={} base-height={} intrinsic-min-height={} intrinsic-max-height={} minimum-height={} used-height={}",
                            item.childId,
                            isBodyFlex,
                            describeSizeState(item.flexBaseSize),
                            describeSizeState(item.mainIntrinsicSizes.minimum),
                            describeSizeState(item.mainIntrinsicSizes.maximum),
                            describeSizeState(item.minimumMainSize),
                            item.usedMainSize
                        );
                    }
                }
            }
        }

        if (!flex.axis.isRow && mutate) {
            for (const auto& line : flex.lines) {
                for (const auto& item : line.items) {
                    if (item.childId == 104) {
                        std::println(
                            "[RightPane height allocation] container={} available-height={} total-base-height={} resolved-total-height={} node104-base-height={} node104-intrinsic-min-height={} node104-intrinsic-max-height={} node104-minimum-height={} node104-used-height={} intrinsic-height-request={}",
                            node->id,
                            describeSizeState(availableSize.height),
                            totalFlexBaseSize,
                            resolvedMainSizes.overallTotalAfter,
                            describeSizeState(item.flexBaseSize),
                            describeSizeState(item.mainIntrinsicSizes.minimum),
                            describeSizeState(item.mainIntrinsicSizes.maximum),
                            describeSizeState(item.minimumMainSize),
                            item.usedMainSize,
                            intrinsicHeightRequest.has_value()
                        );
                    }
                }
            }
        }

        // if (flex.axis.isRow) {
        //     for (const auto& line : flex.lines) {
        //         for (const auto& item : line.items) {
        //             const auto* child = node->children[item.childIndex].get();
        //             bool isRightPane = false;
        //             for (const auto& grandchild : child->children) {
        //                 if (grandchild->id == 104) {
        //                     isRightPane = true;
        //                     break;
        //                 }
        //             }
        //
        //             if (isRightPane) {
        //                 std::println(
        //                     "[BodyFlex width allocation] container={} right-pane={} available-width={} base-width={} intrinsic-min-width={} minimum-width={} used-width={}",
        //                     node->id,
        //                     item.childId,
        //                     describeSizeState(flex.axis.mainSize(availableSize)),
        //                     describeSizeState(item.flexBaseSize),
        //                     describeSizeState(item.mainIntrinsicSizes.minimum),
        //                     describeSizeState(item.minimumMainSize),
        //                     item.usedMainSize
        //                 );
        //             }
        //         }
        //     }
        // }
    }

    FlexResolver::FlexResult FlexResolver::phaseC() {
        float minimumCrossContribution = 0.0f;
        float maximumCrossContribution = 0.0f;

        for (auto& line : flex.lines) {
            line.maxCrossSize = 0.0f;
            float lineMinimumCrossContribution = 0.0f;
            float lineMaximumCrossContribution = 0.0f;

            // const bool containsNode104 = std::ranges::any_of(
            //     line.items,
            //     [](const FlexItem& item) { return item.childId == 104; }
            // );

            // if (containsNode104) {
            //     lineMinimumCrossContribution = 0.0f;
            //     lineMaximumCrossContribution = 100.0f;
            //     line.maxCrossSize = 100.0f;
            // }else {
            for (auto& item : line.items) {
                auto childNode = node->children[item.childIndex].get();
                auto preparedChildConstraints = prepareChildConstraints();
                Measured childMeasured = *childNode->measured;

                // if (item.childId == 104) {
                //     std::println("prepared child constraints aw: {} ah: {}", describeSize(preparedChildConstraints.availableWidth), describeSize(preparedChildConstraints.availableHeight));
                // }

                SizePair childAvailableSize = availableSize;
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


                LayoutOutput childOutput = tree.layoutRecursive(childNode, frameInfo, preparedChildConstraints, childMeasured, false, childRequest);
                const SizeResult& sizeResult = childOutput.sizeResult;

                const float* preferredCrossSize = std::get_if<float>(&flex.axis.crossSize(sizeResult.outerSize));

                item.hypotheticalCrossSize = preferredCrossSize ? *preferredCrossSize : flex.axis.crossSize(childOutput.layout);

                const auto& measuredCrossIntrinsicSizes = flex.axis.isRow ? sizeResult.heightIntrinsicSizes : sizeResult.widthIntrinsicSizes;

                float childMinimumCrossContribution = std::get<float>(measuredCrossIntrinsicSizes->minimum);
                float childMaximumCrossContribution = std::get<float>(measuredCrossIntrinsicSizes->maximum);

                if (node->id == 21 && flex.axis.isRow && intrinsicHeightRequest) {
                    std::println(
                        "[Body intrinsic height child] child={} available-height={} hypothetical-height={} intrinsic-min-height={} intrinsic-max-height={}",
                        item.childId,
                        describeSizeState(childRequest.available.height),
                        item.hypotheticalCrossSize,
                        childMinimumCrossContribution,
                        childMaximumCrossContribution
                    );
                }

                // if (containsNode104 && intrinsicWidthRequest) {
                //     std::println(
                //         "[RightPaneFlex intrinsic width] container={} child={} available-width={} hypothetical-width={} intrinsic-min-width={} intrinsic-max-width={}",
                //         node->id,
                //         item.childId,
                //         describeSizeState(childRequest.available.width),
                //         item.hypotheticalCrossSize,
                //         childMinimumCrossContribution,
                //         childMaximumCrossContribution
                //     );
                // }

                lineMinimumCrossContribution = std::max(lineMinimumCrossContribution, childMinimumCrossContribution);
                lineMaximumCrossContribution = std::max(lineMaximumCrossContribution, childMaximumCrossContribution);
                line.maxCrossSize = std::max(line.maxCrossSize, item.hypotheticalCrossSize);
            }

            // if (containsNode104 && intrinsicWidthRequest) {
            //     std::println(
            //         "[RightPaneFlex intrinsic width] container={} collected-min-width={} collected-max-width={} natural-line-width={}",
            //         node->id,
            //         lineMinimumCrossContribution,
            //         lineMaximumCrossContribution,
            //         line.maxCrossSize
            //     );
            // }

            if (node->id == 21 && flex.axis.isRow && intrinsicHeightRequest) {
                std::println(
                    "[Body intrinsic height collected] minimum-height={} maximum-height={} natural-line-height={}",
                    lineMinimumCrossContribution,
                    lineMaximumCrossContribution,
                    line.maxCrossSize
                );
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
            [&](const auto&) { return contentCrossSize; },
        }, flex.axis.crossSize(availableSize));
        // float availableCross = contentCrossSize;

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
            SizePair childAvailableSize = availableSize;

            // if (childNode->id == 104)
            //     std::println("placement cross size: {}", placement.lineCrossSize);

            flex.axis.mainSize(childAvailableSize) = placement.mainSize;
            flex.axis.crossSize(childAvailableSize) = placement.lineCrossSize;


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
            //     std::println("avail in phase C: {} spec in phase C: {}", describeSizeState(childRequest.available.width), describeSizeState(childRequest.specified.width));
            // }


            // this main size is getting miscomputed... somehow
            flex.axis.mainSize(childRequest.override) = placement.mainSize;


            // the cross axis where this child node exists is also wrong butl ike jfc HOW DO I DO THIS BRO
            // if (childNode->id == 104) {
            //     std::println("override: {}", describeSizeState(flex.axis.crossSize(childRequest.override)));
            // }
            // why does an override not impact the actual size?

            if (flex.axis.isRow) {
                childRequest.automaticHeight = placement.alignment == AlignItems::Stretch ? AutomaticSizing::UseAvailable : AutomaticSizing::UseContent;
            } else {
                childRequest.automaticWidth = placement.alignment == AlignItems::Stretch ? AutomaticSizing::UseAvailable : AutomaticSizing::UseContent;
            }

            if (mutate && flex.axis.isRow && childNode->id == 86) {
                std::println(
                    "[Body to RightPane height handoff] container={} body-available-height={} line-height={} request-available-height={} specified-height={} mutate={} intrinsic-height-request={}",
                    node->id,
                    describeSizeState(availableSize.height),
                    placement.lineCrossSize,
                    describeSizeState(childRequest.available.height),
                    describeSizeState(childRequest.specified.height),
                    mutate,
                    childRequest.intrinsicHeightRequest.has_value()
                );
            }

            if (mutate && !flex.axis.isRow && childNode->id == 104) {
                std::println(
                    "[node 104 final height request] container={} placement-height={} available-height={} override-height={} specified-height={} mutate={} intrinsic-height-request={}",
                    node->id,
                    placement.mainSize,
                    describeSizeState(childRequest.available.height),
                    describeSizeState(childRequest.override.height),
                    describeSizeState(childRequest.specified.height),
                    mutate,
                    childRequest.intrinsicHeightRequest.has_value()
                );
            }

            
            LayoutOutput childOutput = tree.layoutRecursive(childNode, frameInfo, preparedChildConstraints, childMeasured, mutate, childRequest);
            const auto& childLayout = childOutput.layout;

            if (mutate && flex.axis.isRow && childNode->id == 86) {
                std::println(
                    "[Body to RightPane height result] container={} line-height={} result-outer-height={} computed-box-height={} consumed-height={} mutate={}",
                    node->id,
                    placement.lineCrossSize,
                    describeSizeState(childOutput.sizeResult.outerSize.height),
                    childLayout.computedBox.height,
                    childLayout.consumedHeight,
                    mutate
                );
            }

            if (mutate && !flex.axis.isRow && childNode->id == 104) {
                std::println(
                    "[node 104 final height result] container={} requested-placement-height={} result-outer-height={} computed-box-height={} consumed-height={} mutate={}",
                    node->id,
                    placement.mainSize,
                    describeSizeState(childOutput.sizeResult.outerSize.height),
                    childLayout.computedBox.height,
                    childLayout.consumedHeight,
                    mutate
                );
            }

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
