
#pragma once

#include "new_arch.hpp"
#include "element.hpp"
#include "new_sizing.hpp"
#include "sizing.hpp"
#include <print>
#include <variant>

namespace tree {
    struct RenderTree;
}

namespace layout {
    using style::AlignContent;
    using style::AlignItems;
    using style::AlignSelf;
    using style::FlexDirection;
    using style::FlexWrap;
    using style::JustifyContent;
    using style::Overflow;
    using style::Size;
    using style::SizeError;
    using style::Unit;
    using tree::RenderTree;
    using tree::TreeNode;

    struct AxisHelper {
        bool isRow;
        bool isReversed;

        AxisHelper() {};

        AxisHelper(FlexDirection dir):
            isRow{dir == FlexDirection::Row || dir == FlexDirection::RowReverse},
            isReversed{dir == FlexDirection::RowReverse || dir == FlexDirection::ColReverse}
        {}

        void applyDirection(Direction dir) {
            if (dir == Direction::rtl && isRow) isReversed = !isReversed;
        }

        float mainSize(const LayoutResult& lr) {
            return isRow ? lr.computedBox.width : lr.computedBox.height;
        }

        float crossSize(const LayoutResult& lr) {
            return isRow ? lr.computedBox.height : lr.computedBox.width;
        }
        const SizeState& mainSize(const SizePair& size) {
            return isRow ? size.width : size.height;
        }
        const SizeState& crossSize(const SizePair& size) {
            return isRow ? size.height : size.width;
        }
        SizeState& mainSize(SizePair& size) {
            return isRow ? size.width : size.height;
        }
        SizeState& crossSize(SizePair& size) {
            return isRow ? size.height : size.width;
        }
        simd_float2 toPhysical(float main, float cross) {
            return isRow
                ? simd_float2{main, cross}
                : simd_float2{cross, main};
        }

    };


    struct FlexItem {
        size_t childIndex;
        uint64_t childId;
        SizeState flexBaseSize;
        float hypotheticalMainSize;
        SizeState minimumMainSize;
        SizeState maximumMainSize;
        IntrinsicResult mainIntrinsicSizes;
        float flexGrow;
        float scaledFlexShrink;
        AlignItems alignment;
        float usedMainSize;
        float hypotheticalCrossSize;
    };

    struct FlexLine {
        std::vector<FlexItem> items;
        float maxCrossSize{};

        void addItem(FlexItem item) {
            items.push_back(item);
        }

        size_t count() { return items.size(); }

        float totalWithGap(float gap) {
            float total = 0.0f;
            for (const auto& item : items) total += std::get<float>(item.flexBaseSize);
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        float totalHypotheticalWithGap(float gap) {
            float total = 0.0f;
            for (const auto& item : items) total += item.hypotheticalMainSize;
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        float totalMinimumContributionWithGap(float gap) {
            float total = 0.0f;
            for (const auto& item : items) total += std::get<float>(item.mainIntrinsicSizes.minimum);
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        float totalMaximumContributionWithGap(float gap) {
            float total = 0.0f;
            for (const auto& item : items) total += std::get<float>(item.mainIntrinsicSizes.maximum);
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        struct ResolveResult {
            float totalAfter{};
            float remainingSpace{};
        };

        ResolveResult resolve(float availableMain) {
            ResolveResult result;
            std::vector<bool> frozen(items.size(), false);

            for (auto& item : items)
                item.usedMainSize = std::get<float>(item.flexBaseSize);

            // flex redistribution algo            
            while (true) {
                float frozenTotal = 0.0f;
                float unfrozenBaseTotal = 0.0f;
                float unfrozenGrowthTotal = 0.0f;
                float unfrozenShrinkTotal = 0.0f;

                float totalViolation = 0.0f;
                std::vector<bool> minimumViolations(items.size(), false);
                std::vector<bool> maximumViolations(items.size(), false);

                for (size_t i = 0; i < items.size(); ++i) {
                    auto& item = items[i];
                    if (frozen[i]) {
                        frozenTotal += item.usedMainSize;
                    } else {
                        unfrozenBaseTotal += std::get<float>(item.flexBaseSize);
                        unfrozenGrowthTotal += item.flexGrow;
                        unfrozenShrinkTotal += item.scaledFlexShrink;
                    }
                }

                float space = availableMain - frozenTotal - unfrozenBaseTotal;

                // redistribute space among unfrozen items
                for (size_t i = 0; i < items.size(); ++i) {
                    if (frozen[i]) 
                        continue;
                    
                    auto& item = items[i];

                    if (space > 0.0f && item.flexGrow > 0.0f && unfrozenGrowthTotal > 0.0f) {
                        item.usedMainSize = std::get<float>(item.flexBaseSize) + (item.flexGrow / unfrozenGrowthTotal) * space;
                    } else if (space < 0.0f && item.scaledFlexShrink > 0.0f && unfrozenShrinkTotal > 0.0f) {
                        item.usedMainSize = std::get<float>(item.flexBaseSize) + (item.scaledFlexShrink / unfrozenShrinkTotal) * space;
                    } else {
                        item.usedMainSize = std::get<float>(item.flexBaseSize);
                    }
                };

                // find violators
                for (size_t i = 0; i < items.size(); ++i) {
                    auto& item = items[i];

                    float clamped = item.usedMainSize;
                    if (std::holds_alternative<float>(item.maximumMainSize)) {
                        clamped = std::min(clamped, std::get<float>(item.maximumMainSize));
                    }

                    float minMainSize = std::get<float>(item.minimumMainSize);

                    clamped = std::max(clamped, std::get<float>(item.minimumMainSize));
                    
                    if (clamped != item.usedMainSize) { 
                        float violation = item.usedMainSize - clamped;
                        totalViolation += violation;

                        minimumViolations[i] = violation > 0.0f;
                        maximumViolations[i] = violation < 0.0f;

                        item.usedMainSize = clamped;
                    }
                }

                // if violation == 0, break (all frozen)
                if (totalViolation == 0.0f) {
                    break;
                }

                // else, freeze according to violation policy
                // positive violation (freeze mins)
                // negative violation (freeze maxes)
                const auto& violations = totalViolation > 0.0f ? minimumViolations : maximumViolations;

                for (size_t i = 0; i < items.size(); ++i) {
                    if (violations[i]) {
                        frozen[i] = true;
                    }
                }
            }

            for (const auto& item : items) result.totalAfter += item.usedMainSize;
            result.remainingSpace = availableMain - result.totalAfter;
            return result;
        }
    };

    struct Alignment {
        float initialOffset{};
        float spaceBetween{};
    };

    enum class DistributeMode {
        FlexStart, FlexEnd, Center, SpaceBetween, SpaceAround, SpaceEvenly
    };

    Alignment distributeSpace(float remainingSpace, size_t itemCount, DistributeMode mode);

    DistributeMode toDistributeMode(JustifyContent jc);

    DistributeMode toDistributeMode(AlignContent ac);


    struct FlexLayout {
        AxisHelper axis;
        JustifyContent justifyContent;
        AlignItems alignItems;
        AlignContent alignContent;
        FlexWrap flexWrap;

        std::vector<FlexLine> lines;
        FlexLine currentLine;

        struct ChildPlacement {
            size_t childIndex;
            float mainOffset;
            float crossOffset;
            float mainSize;
            float lineCrossSize;
            AlignItems alignment;
        };

        FlexLayout() {}

        FlexLayout(FlexDirection dir, JustifyContent jc, AlignItems ai,
                    AlignContent ac, FlexWrap wrap):
            axis{dir}, justifyContent{jc}, alignItems{ai},
            alignContent{ac}, flexWrap{wrap}
        {}

        AlignItems effectiveAlign(AlignSelf selfAlign) {
            switch (selfAlign) {
                case AlignSelf::Auto:      return alignItems;
                case AlignSelf::Stretch:   return AlignItems::Stretch;
                case AlignSelf::FlexStart: return AlignItems::FlexStart;
                case AlignSelf::FlexEnd:   return AlignItems::FlexEnd;
                case AlignSelf::Center:    return AlignItems::Center;
            }
        }

        void addItem(
            size_t childIndex,
            TreeNode* child,
            SizeState flexBaseSize,
            SizeState minimumMainSize,
            SizeState maximumMainSize,
            IntrinsicResult mainIntrinsicSizes,
            AlignItems alignment,
            const SizeState& availableMain,
            float gap
        ) {
            float hypotheticalMainSize = std::get<float>(flexBaseSize);
            if (std::holds_alternative<float>(maximumMainSize)) {
                hypotheticalMainSize = std::min(hypotheticalMainSize, std::get<float>(maximumMainSize));
            }
            hypotheticalMainSize = std::max(hypotheticalMainSize, std::get<float>(minimumMainSize));

            const float* availableMainSize = std::get_if<float>(&availableMain);
            if (flexWrap != FlexWrap::NoWrap && currentLine.count() > 0 && availableMainSize) {
                if (currentLine.totalHypotheticalWithGap(gap) + gap + hypotheticalMainSize > *availableMainSize) {
                    lines.push_back(currentLine);
                    currentLine = FlexLine{};
                }
            }

            float grow = child->getFlexGrow().resolveOr(Size::px(0.0f), 0.0f);
            float shrink = child->getFlexShrink().resolveOr(Size::px(0.0f), 1.0f);
            float scaledFlexShrink = shrink > 0.0f ? std::get<float>(flexBaseSize) * shrink : 0.0f;
            float usedMainSize = std::get<float>(flexBaseSize);
            currentLine.addItem({
                .childIndex = childIndex,
                .childId = child->id,
                .flexBaseSize = flexBaseSize,
                .hypotheticalMainSize = hypotheticalMainSize,
                .minimumMainSize = minimumMainSize,
                .maximumMainSize = maximumMainSize,
                .mainIntrinsicSizes = mainIntrinsicSizes,
                .flexGrow = grow > 0.0f ? grow : 0.0f,
                .scaledFlexShrink = scaledFlexShrink,
                .alignment = alignment,
                .usedMainSize = usedMainSize,
                .hypotheticalCrossSize = 0.0f
            });
        }

        struct ResolveResult {
            std::vector<float> lineTotalsAfter;       
            float overallTotalAfter{};
        };

        ResolveResult resolveSizes(float avMain, float gap = 0.0f) {
            ResolveResult result;
            for (auto& line : lines) {
                float lineGap = line.count() > 1 ? gap * (line.count() - 1) : 0.0f;
                auto lr = line.resolve(avMain - lineGap);
                result.overallTotalAfter += lr.totalAfter;
                result.lineTotalsAfter.push_back(lr.totalAfter);
            }
            return result;
        }

        std::vector<ChildPlacement> computePlacements(
            const ResolveResult& resolved,
            float availableMain,
            float availableCross,
            float gap
        ) {
            size_t lineCount = lines.size();

            std::vector<float> lineCrossSizes(lineCount);
            std::vector<float> lineCrossOffsets(lineCount);

            if (lineCount == 1) {
                lineCrossSizes[0] = availableCross;
                lineCrossOffsets[0] = 0.0f;
            } else {
                float totalNaturalCross = 0;
                for (size_t li = 0; li < lineCount; ++li) {
                    lineCrossSizes[li] = lines[li].maxCrossSize;
                    totalNaturalCross += lines[li].maxCrossSize;
                }

                float crossGap = gap * (lineCount - 1);
                float remainingCross = availableCross - totalNaturalCross - crossGap;

                if (alignContent == AlignContent::Stretch && remainingCross > 0) {
                    float extra = remainingCross / lineCount;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossSizes[li] += extra;
                    }
                    float crossAccum = 0;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li] + gap;
                    }
                } else {
                    auto crossAlign = distributeSpace(
                        remainingCross, lineCount, toDistributeMode(alignContent));
                    float crossAccum = crossAlign.initialOffset;
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = crossAccum;
                        crossAccum += lineCrossSizes[li] + crossAlign.spaceBetween + gap;
                    }
                }

                // Wrap-reverse: mirror line cross offsets
                if (flexWrap == FlexWrap::WrapReverse) {
                    for (size_t li = 0; li < lineCount; ++li) {
                        lineCrossOffsets[li] = availableCross - lineCrossOffsets[li] - lineCrossSizes[li];
                    }
                }
            }

            // Build per-child placements
            std::vector<ChildPlacement> placements;
            for (size_t li = 0; li < lineCount; ++li) {
                auto& line = lines[li];
                float lineGap = line.count() > 1 ? gap * (line.count() - 1) : 0.0f;
                float lineRemainingMain = availableMain - resolved.lineTotalsAfter[li] - lineGap;
                auto mainAlign = distributeSpace(lineRemainingMain, line.count(), toDistributeMode(justifyContent));

                float accumulated = mainAlign.initialOffset;
                float lineCross = lineCrossSizes[li];
                float lineCrossBase = lineCrossOffsets[li];

                for (size_t ci = 0; ci < line.count(); ++ci) {
                    auto& item = line.items[ci];
                    ChildPlacement placement;
                    placement.childIndex = item.childIndex;
                    placement.mainOffset = accumulated;
                    placement.mainSize = item.usedMainSize;
                    placement.lineCrossSize = lineCross;
                    placement.alignment = item.alignment;

                    // if (item.childId == 104) {
                    //     placement.lineCrossSize = 100.0;
                    // }

                    float childCrossSize = item.hypotheticalCrossSize;

                    switch (item.alignment) {
                        case AlignItems::Stretch:
                            placement.crossOffset = lineCrossBase;
                            break;
                        case AlignItems::FlexStart:
                            placement.crossOffset = lineCrossBase;
                            break;
                        case AlignItems::Center:
                            placement.crossOffset = lineCrossBase + (lineCross - childCrossSize) / 2.0f;
                            break;
                        case AlignItems::FlexEnd:
                            placement.crossOffset = lineCrossBase + lineCross - childCrossSize;
                            break;
                    }

                    if (axis.isReversed) {
                        placement.mainOffset = availableMain - accumulated - placement.mainSize;
                    }

                    accumulated += item.usedMainSize + mainAlign.spaceBetween + gap;
                    placements.push_back(placement);
                }
            }

            return placements;
        }
    };

    struct FlexResolver {
        RenderTree& tree;
        TreeNode* node;
        Constraints parentConstraints;
        Constraints childConstraints;
        FlexLayout flex;
        const FrameInfo& frameInfo;
        const SizePair& availableSize;
        bool mutate;
        std::unordered_map<size_t, SizeResult>& sizeCache;


        float minX;
        float minY;
        float maxX;
        float maxY;

        std::optional<IntrinsicRequest> intrinsicWidthRequest;
        std::optional<IntrinsicRequest> intrinsicHeightRequest;

        float resolvedGap{};
        float availableMain{};
        FlexLayout::ResolveResult resolvedMainSizes;

        struct Bounds {
            float maxX;
            float maxY;
        };

        struct FlexResult {
            Bounds bounds;
            IntrinsicResult mainIntrinsicSizes;
            IntrinsicResult crossIntrinsicSizes;
        };

        FlexResolver(RenderTree& tree, TreeNode* node, const Constraints& parentConstraints,
                        const Constraints& childConstraints, FlexLayout flex, const FrameInfo& frameInfo,
                        const SizePair& availableSize, bool mutate, std::unordered_map<size_t, SizeResult>& sizeCache,
                        float minX, float minY, float maxX, float maxY,
                        std::optional<IntrinsicRequest> intrinsicWidthRequest, std::optional<IntrinsicRequest> intrinsicHeightRequest)
            : tree{tree}, node{node}, parentConstraints{parentConstraints},
                childConstraints{childConstraints}, flex{flex},
                frameInfo{frameInfo}, availableSize{availableSize}, mutate{mutate}, sizeCache{sizeCache},
                minX{minX}, minY{minY}, maxX{maxX}, maxY{maxY},
                intrinsicWidthRequest{intrinsicWidthRequest}, intrinsicHeightRequest{intrinsicHeightRequest}
        {}

        Constraints prepareChildConstraints();
        void phaseB();
        FlexResult phaseC();
    };
}
