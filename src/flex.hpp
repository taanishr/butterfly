
#pragma once

#include "new_arch.hpp"
#include "element.hpp"
#include "sizing.hpp"

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
        const Size& mainSize(const SharedDescriptor& shared) {
            return isRow ? shared.width : shared.height;
        }
        const Size& crossSize(const SharedDescriptor& shared) {
            return isRow ? shared.height : shared.width;
        }
        const Size& minMainSize(const SharedDescriptor& shared) {
            return isRow ? shared.minWidth : shared.minHeight;
        }
        const std::optional<Size>& maxMainSize(const SharedDescriptor& shared) {
            return isRow ? shared.maxWidth : shared.maxHeight;
        }
        const Size& minCrossSize(const SharedDescriptor& shared) {
            return isRow ? shared.minHeight : shared.minWidth;
        }
        const std::optional<Size>& maxCrossSize(const SharedDescriptor& shared) {
            return isRow ? shared.maxHeight : shared.maxWidth;
        }

    std::optional<AxisResolution>& mainResolution(Constraints& c) {
            return isRow ? c.widthResolution : c.heightResolution;
        }

        std::optional<AxisResolution>& crossResolution(Constraints& c) {
            return isRow ? c.heightResolution : c.widthResolution;
        }
        

        simd_float2 toPhysical(float main, float cross) {
            return isRow
                ? simd_float2{main, cross}
                : simd_float2{cross, main};
        }

        Size& mainAvailable(Constraints& c) {
            return isRow ? c.availableWidth : c.availableHeight;
        }

        Size& crossAvailable(Constraints& c) {
            return isRow ? c.availableHeight : c.availableWidth;
        }
    };


    struct FlexItem {
        size_t childIndex;
        float flexBaseSize;
        float hypotheticalMainSize;
        float minMainSize;
        std::optional<float> maxMainSize;
        float flexGrow;
        float scaledFlexShrink;
        AlignItems alignment;
        float usedMainSize;
        float hypotheticalCrossSize;
    };

    struct FlexLine {
        std::vector<FlexItem> items;
        float maxCrossSize{};
        float intrinsicMinCrossSize{};
        float intrinsicMaxCrossSize{};

        void addItem(FlexItem item) {
            items.push_back(std::move(item));
        }

        size_t count() { return items.size(); }

        float totalWithGap(float gap) {
            float total = 0.0f;
            for (const auto& item : items) total += item.flexBaseSize;
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        float totalHypotheticalWithGap(float gap) {
            float total = 0.0f;
            for (const auto& item : items) total += item.hypotheticalMainSize;
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        float totalMinimumWithGap(float gap) {
            float total = 0.0f;
            for (const auto& item : items) total += item.minMainSize;
            return total + (count() > 1 ? gap * (count() - 1) : 0.0f);
        }

        struct ResolveResult {
            float totalAfter{};
            float remainingSpace{};
        };

        ResolveResult resolve(float availableMain) {
            ResolveResult result;
            std::vector<bool> frozen(items.size(), false);
            for (auto& item : items) item.usedMainSize = item.flexBaseSize;

            while (true) {
                float frozenTotal = 0.0f;
                float unfrozenBaseTotal = 0.0f;
                float unfrozenGrowthTotal = 0.0f;
                float unfrozenShrinkTotal = 0.0f;

                for (size_t i = 0; i < items.size(); ++i) {
                    auto& item = items[i];
                    if (frozen[i]) {
                        frozenTotal += item.usedMainSize;
                    } else {
                        unfrozenBaseTotal += item.flexBaseSize;
                        unfrozenGrowthTotal += item.flexGrow;
                        unfrozenShrinkTotal += item.scaledFlexShrink;
                    }
                }

                float space = availableMain - frozenTotal - unfrozenBaseTotal;

                for (size_t i = 0; i < items.size(); ++i) {
                    if (frozen[i]) continue;
                    auto& item = items[i];

                    if (space > 0.0f && item.flexGrow > 0.0f && unfrozenGrowthTotal > 0.0f) {
                        item.usedMainSize = item.flexBaseSize + (item.flexGrow / unfrozenGrowthTotal) * space;
                    } else if (space < 0.0f && item.scaledFlexShrink > 0.0f && unfrozenShrinkTotal > 0.0f) {
                        item.usedMainSize = item.flexBaseSize + (item.scaledFlexShrink / unfrozenShrinkTotal) * space;
                    } else {
                        item.usedMainSize = item.flexBaseSize;
                    }
                }

                bool anyViolation = false;

                for (size_t i = 0; i < items.size(); ++i) {
                    if (frozen[i]) continue;
                    auto& item = items[i];

                    float clamped = item.usedMainSize;
                    if (item.maxMainSize.has_value()) {
                        clamped = std::min(clamped, *item.maxMainSize);
                    }
                    clamped = std::max(clamped, item.minMainSize);

                    if (clamped != item.usedMainSize) {
                        item.usedMainSize = clamped;
                        frozen[i] = true;
                        anyViolation = true;
                    }
                }

                if (!anyViolation) break;
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
            float flexBaseSize,
            float minMainSize,
            std::optional<float> maxMainSize,
            AlignItems alignment,
            const SizeState& availableMain,
            float gap
        ) {
            float hypotheticalMainSize = flexBaseSize;
            if (maxMainSize.has_value()) {
                hypotheticalMainSize = std::min(hypotheticalMainSize, *maxMainSize);
            }
            hypotheticalMainSize = std::max(hypotheticalMainSize, minMainSize);

            const float* availableMainSize = std::get_if<float>(&availableMain);
            if (flexWrap != FlexWrap::NoWrap && currentLine.count() > 0 && availableMainSize) {
                if (currentLine.totalHypotheticalWithGap(gap) + gap + hypotheticalMainSize > *availableMainSize) {
                    lines.push_back(std::move(currentLine));
                    currentLine = FlexLine{};
                }
            }

            float grow = child->getFlexGrow().resolveOr(Size::px(0.0f), 0.0f);
            float shrink = child->getFlexShrink().resolveOr(Size::px(0.0f), 1.0f);
            currentLine.addItem({
                .childIndex = childIndex,
                .flexBaseSize = flexBaseSize,
                .hypotheticalMainSize = hypotheticalMainSize,
                .minMainSize = minMainSize,
                .maxMainSize = maxMainSize,
                .flexGrow = grow > 0.0f ? grow : 0.0f,
                .scaledFlexShrink = shrink > 0.0f ? flexBaseSize * shrink : 0.0f,
                .alignment = alignment,
                .usedMainSize = flexBaseSize,
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
                    ChildPlacement p;
                    p.childIndex = item.childIndex;
                    p.mainOffset = accumulated;
                    p.mainSize = item.usedMainSize;
                    p.lineCrossSize = lineCross;
                    p.alignment = item.alignment;

                    float childCrossSize = item.hypotheticalCrossSize;

                    switch (item.alignment) {
                        case AlignItems::Stretch:
                            p.crossOffset = lineCrossBase;
                            break;
                        case AlignItems::FlexStart:
                            p.crossOffset = lineCrossBase;
                            break;
                        case AlignItems::Center:
                            p.crossOffset = lineCrossBase + (lineCross - childCrossSize) / 2.0f;
                            break;
                        case AlignItems::FlexEnd:
                            p.crossOffset = lineCrossBase + lineCross - childCrossSize;
                            break;
                    }

                    if (axis.isReversed) {
                        p.mainOffset = availableMain - accumulated - p.mainSize;
                    }

                    accumulated += item.usedMainSize + mainAlign.spaceBetween + gap;
                    placements.push_back(p);
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
        Measured measured;
        bool mutate;

        float minX;
        float minY;
        float maxX;
        float maxY;
        float resolvedGap{};
        float availableMain{};
        FlexLayout::ResolveResult resolvedMainSizes;
        std::optional<IntrinsicSizes> intrinsicSizes;

        struct Bounds {
            float maxX;
            float maxY;
        };

        FlexResolver(RenderTree& tree, TreeNode* node, const Constraints& parentConstraints,
                        const Constraints& childConstraints, FlexLayout flex, const FrameInfo& frameInfo,
                        const SizePair& availableSize, Measured measured, bool mutate,
                        float minX, float minY, float maxX, float maxY)
            : tree{tree}, node{node}, parentConstraints{parentConstraints},
                childConstraints{childConstraints}, flex{std::move(flex)},
                frameInfo{frameInfo}, availableSize{availableSize}, measured{measured}, mutate{mutate},
                minX{minX}, minY{minY}, maxX{maxX}, maxY{maxY}
        {        }

        Constraints prepareChildConstraints(TreeNode* child);
        void phaseB();
        Bounds phaseC();
    };
}
