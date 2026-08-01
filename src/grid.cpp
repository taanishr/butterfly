#include "grid.hpp"
#include "render_tree.hpp"
#include <algorithm>
#include <optional>

namespace layout {
    namespace {
        float applyMinMax(float value, const Size& minSize, const std::optional<Size>& maxSize, float referenceSize) {
            auto basis = Size::px(referenceSize);

            if (maxSize.has_value()) {
                auto resolvedMax = maxSize->resolve(basis);
                if (resolvedMax) {
                    value = std::min(value, *resolvedMax);
                }
            }

            auto resolvedMin = minSize.resolve(basis);
            if (resolvedMin) {
                value = std::max(value, *resolvedMin);
            }

            return value;
        }

        void distributeSpanningContribution(std::vector<float>& contributions, const std::vector<float>& fixedSizes, const std::vector<bool>& fixedTracks, const std::vector<Size>& definitions, size_t start, size_t end, float gap, float contribution) {
            float covered = gap * static_cast<float>(end - start - 1);
            bool spansFractionTrack = false;
            for (size_t i = start; i < end; ++i) {
                covered += fixedTracks[i] ? fixedSizes[i] : contributions[i];
                spansFractionTrack = spansFractionTrack || definitions[i].isFr();
            }

            float extra = contribution - covered;
            if (extra <= 0.0f)
                return;

            float totalWeight = 0.0f;
            size_t eligibleTracks = 0;
            for (size_t i = start; i < end; ++i) {
                if (fixedTracks[i] || (spansFractionTrack && !definitions[i].isFr()))
                    continue;
                totalWeight += spansFractionTrack ? definitions[i].value : 1.0f;
                eligibleTracks++;
            }
            if (eligibleTracks == 0)
                return;

            for (size_t i = start; i < end; ++i) {
                if (fixedTracks[i] || (spansFractionTrack && !definitions[i].isFr()))
                    continue;
                float weight = spansFractionTrack && totalWeight > 0.0f ? definitions[i].value / totalWeight : 1.0f / static_cast<float>(eligibleTracks);
                contributions[i] += extra * weight;
            }
        }
    }

    void GridLayout::addChild(size_t childIndex, TreeNode* node, GridItemContributions widthContributions) {
        auto gridPlacement = node->getGridPlacement();

        std::optional<int> cs, ce, rs, re;

        if (gridPlacement.colStart != 0) {
            cs = gridPlacement.colStart - 1;

            if (gridPlacement.colEnd != 0) {
                ce = gridPlacement.colEnd - 1;
            }else {
                ce = *cs + 1;
            }
        }

        if (gridPlacement.rowStart != 0) {
            rs = gridPlacement.rowStart - 1;

            if (gridPlacement.rowEnd != 0) {
                re = gridPlacement.rowEnd - 1;
            }else {
                re = *rs + 1;
            }
        }

        items.push_back({
            .childIndex = childIndex,
            .placement = {.colStart = cs, .colEnd = ce, .rowStart = rs, .rowEnd = re},
            .widthContributions = widthContributions,
            .heightContributions = {}
        });
    }

    Grid::Grid(size_t rows, size_t cols, GridDirection major): 
        occupied{rows, std::vector<uint8_t>(cols, 0)},
        numRows{rows},
        numCols{cols}, 
        majorAxis{major}
    {}

    void Grid::mark(int row, int col) {
        occupied[row][col] = 1;
    }

    bool Grid::regionFree(int row, int col, int spanRows, int spanCols) const {
        for (int r = row; r < row + spanRows; ++r)
            for (int c = col; c < col + spanCols; ++c)
                if (occupied[r][c]) return false;

        
        return true;
    }

    void Grid::growMajor(int needed) {
        if (majorAxis == GridDirection::Row) {
            while (numRows < needed) {
                occupied.push_back(std::vector<uint8_t>(numCols, 0));
                numRows++;
            }
        } else {
            while (numCols < needed) {
                for (auto& row : occupied)
                    row.push_back(0);
                numCols++;
            }
        }
    }

    void Grid::advanceCursor(int spanMinor) {
        cursorMinor += spanMinor;

        if (cursorMinor >= minorSize()) {
            cursorMinor = 0;
            cursorMajor++;
        }

    }

    int Grid::majorSize() const {
        return (majorAxis == GridDirection::Row) ? numRows : numCols;
    }

    int Grid::minorSize() const {
        return (majorAxis == GridDirection::Row) ? numCols : numRows;
    }

    std::pair<int, int> Grid::findSpace(int spanRows, int spanCols) {
        bool rowMajor = (majorAxis == GridDirection::Row);
        int spanMajor = rowMajor ? spanRows : spanCols;
        int spanMinor = rowMajor ? spanCols : spanRows;

        while (true) {
            if (cursorMinor + spanMinor > minorSize()) {
                cursorMinor = 0;
                cursorMajor++;
            }
            
            if (cursorMajor + spanMajor > majorSize()) {
                growMajor(cursorMajor + spanMajor);
            }

            int row = rowMajor ? cursorMajor : cursorMinor;
            int col = rowMajor ? cursorMinor : cursorMajor;

            if (regionFree(row, col, spanRows, spanCols)) {
                advanceCursor(spanMinor);
                return {row, col};
            }

            cursorMinor++;
        }
    }


    void GridLayout::resolveStructure(size_t numRows, size_t numCols) {
        grid = Grid{numRows, numCols};

        // place explicitly placed items : who wins if items conflict in explicit positions?
        for (auto& item : items) {
            auto& placement = item.placement;
            if (!placement.colNeedsResolution() && !placement.rowNeedsResolution()) {
                for (int r = *placement.rowStart; r < *placement.rowEnd; ++r)
                    for (int c = *placement.colStart; c < *placement.colEnd; ++c)
                        grid.mark(r, c);
            }
        }

        // place items with unresolved placements
        for (auto& item : items) {
            auto& placement = item.placement;
            if (!placement.colNeedsResolution() && !placement.rowNeedsResolution()) continue;

            int spanCols = placement.colNeedsResolution() ? 1 : (*placement.colEnd - *placement.colStart);
            int spanRows = placement.rowNeedsResolution() ? 1 : (*placement.rowEnd - *placement.rowStart);

            auto [row, col] = grid.findSpace(spanRows, spanCols);

            if (placement.colNeedsResolution()) {
                placement.colStart = col;
                placement.colEnd = col + spanCols;
            }
            if (placement.rowNeedsResolution()) {
                placement.rowStart = row;
                placement.rowEnd = row + spanRows;
            }

            for (int r = *placement.rowStart; r < *placement.rowEnd; ++r)
                for (int c = *placement.colStart; c < *placement.colEnd; ++c)
                    grid.mark(r, c);
        }
    }


    std::vector<Track> GridLayout::resolveTracks(std::vector<Size>& defs, float available, float gap, bool isCol, bool axisDefinite, IntrinsicSizes* intrinsicSizes) {
        size_t n = defs.size();
        float totalGap = (n > 1) ? gap * (float)(n - 1) : 0;
        float usable = std::max(0.0f, available - totalGap);
        Size basis = axisDefinite ? Size::px(available) : Size::autoSize();
        std::vector<float> fixedSizes(n, 0.0f);
        std::vector<bool> fixedTracks(n, false);
        std::vector<float> minimums(n, 0.0f);
        std::vector<float> minContents(n, 0.0f);
        std::vector<float> maxContents(n, 0.0f);

        for (size_t i = 0; i < n; ++i) {
            auto resolved = defs[i].resolve(basis);
            if (resolved) {
                fixedSizes[i] = *resolved;
                fixedTracks[i] = true;
            }
        }

        size_t maxSpan = 1;
        for (const auto& item : items) {
            const auto& placement = item.placement;
            size_t start = isCol ? *placement.colStart : *placement.rowStart;
            size_t end = isCol ? *placement.colEnd : *placement.rowEnd;
            maxSpan = std::max(maxSpan, end - start);
            if (end - start != 1)
                continue;

            GridItemContributions contributions = isCol ? item.widthContributions : item.heightContributions;
            minimums[start] = std::max(minimums[start], contributions.minimum);
            minContents[start] = std::max(minContents[start], contributions.minContent);
            maxContents[start] = std::max(maxContents[start], contributions.maxContent);
        }

        for (size_t span = 2; span <= maxSpan; ++span) {
            for (const auto& item : items) {
                const auto& placement = item.placement;
                size_t start = isCol ? *placement.colStart : *placement.rowStart;
                size_t end = isCol ? *placement.colEnd : *placement.rowEnd;
                if (end - start != span)
                    continue;

                GridItemContributions contributions = isCol ? item.widthContributions : item.heightContributions;
                bool spansFractionTrack = std::any_of(defs.begin() + start, defs.begin() + end, [](const Size& definition) { return definition.isFr(); });
                if (!spansFractionTrack)
                    distributeSpanningContribution(minimums, fixedSizes, fixedTracks, defs, start, end, gap, contributions.minimum);
                distributeSpanningContribution(minContents, fixedSizes, fixedTracks, defs, start, end, gap, contributions.minContent);
                distributeSpanningContribution(maxContents, fixedSizes, fixedTracks, defs, start, end, gap, contributions.maxContent);
            }
        }

        float intrinsicMin = totalGap;
        float intrinsicMax = totalGap;
        for (size_t i = 0; i < n; ++i) {
            if (fixedTracks[i]) {
                intrinsicMin += fixedSizes[i];
                intrinsicMax += fixedSizes[i];
            } else if (defs[i].unit == style::Unit::MinContent) {
                intrinsicMin += minContents[i];
                intrinsicMax += minContents[i];
            } else if (defs[i].unit == style::Unit::MaxContent) {
                intrinsicMin += maxContents[i];
                intrinsicMax += maxContents[i];
            } else if (defs[i].isFr()) {
                intrinsicMin += minContents[i];
                intrinsicMax += maxContents[i];
            } else {
                intrinsicMin += minContents[i];
                intrinsicMax += maxContents[i];
            }
        }
        if (intrinsicSizes) {
            intrinsicSizes->minContent = Size::px(intrinsicMin);
            intrinsicSizes->maxContent = Size::px(intrinsicMax);
        }

        std::vector<float> sizes(n, 0.0f);
        float nonFractionTotal = 0.0f;
        float fractionTotal = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            if (fixedTracks[i]) {
                sizes[i] = fixedSizes[i];
            } else if (defs[i].unit == style::Unit::MinContent) {
                sizes[i] = minContents[i];
            } else if (defs[i].unit == style::Unit::MaxContent) {
                sizes[i] = maxContents[i];
            } else if (defs[i].unit == style::Unit::FitContent) {
                IntrinsicSizes contributions{.minContent = Size::px(minContents[i]), .maxContent = Size::px(maxContents[i])};
                sizes[i] = resolveIntrinsicSize(defs[i], contributions, basis);
            } else if (defs[i].isFr() && axisDefinite) {
                sizes[i] = minimums[i];
                fractionTotal += defs[i].value;
                continue;
            } else {
                sizes[i] = maxContents[i];
            }
            nonFractionTotal += sizes[i];
        }

        if (fractionTotal > 0.0f) {
            float remainingFractionSpace = std::max(0.0f, usable - nonFractionTotal);
            std::vector<bool> frozen(n, false);
            float unfrozenFractionTotal = fractionTotal;

            while (unfrozenFractionTotal > 0.0f) {
                float fractionSize = remainingFractionSpace / unfrozenFractionTotal;
                bool frozeTrack = false;
                for (size_t i = 0; i < n; ++i) {
                    if (!defs[i].isFr() || frozen[i])
                        continue;
                    float share = defs[i].value * fractionSize;
                    float minimum = minimums[i];
                    if (share >= minimum)
                        continue;
                    sizes[i] = minimum;
                    remainingFractionSpace = std::max(0.0f, remainingFractionSpace - sizes[i]);
                    unfrozenFractionTotal -= defs[i].value;
                    frozen[i] = true;
                    frozeTrack = true;
                }
                if (frozeTrack)
                    continue;
                for (size_t i = 0; i < n; ++i) {
                    if (defs[i].isFr() && !frozen[i])
                        sizes[i] = defs[i].value * fractionSize;
                }
                break;
            }
        }

        std::vector<Track> tracks;
        float offset = 0;
        for (size_t t = 0; t < n; ++t) {
            tracks.push_back({offset, sizes[t]});
            offset += sizes[t] + gap;
        }

        return tracks;
    }

    void GridLayout::resolveColumns(size_t numRows, size_t numCols, const std::vector<Size>& templateCols, float availableWidth, float colGap, bool widthDefinite) {
        resolveStructure(numRows, numCols);
        std::vector<Size> colDefs(grid.numCols, Size::autoSize());

        for (int j = 0; j < templateCols.size(); ++j)
            colDefs[j] = templateCols[j];

        colTracks = resolveTracks(colDefs, availableWidth, colGap, true, widthDefinite, &columnIntrinsicSizes);
    }

    void GridLayout::resolveRows(const std::vector<Size>& templateRows, float availableHeight, float rowGap, bool heightDefinite) {
        std::vector<Size> rowDefs(grid.numRows, Size::autoSize());
        for (int i = 0; i < templateRows.size(); ++i)
            rowDefs[i] = templateRows[i];

        rowTracks = resolveTracks(rowDefs, availableHeight, rowGap, false, heightDefinite, &rowIntrinsicSizes);
    }

    GridResolver::GridResolver(RenderTree& tree, TreeNode* node,
                               const Constraints& parentConstraints,
                               const Constraints& childConstraints,
                               const FrameInfo& frameInfo,
                               Measured measured, bool mutate,
                               Size parentAvailableWidth, Size parentAvailableHeight,
                               float minX, float minY, float maxX, float maxY)
        : tree{tree}, node{node}, parentConstraints{parentConstraints},
          childConstraints{childConstraints},
          alignItems{node->getAlignItems()},
          justifyItems{node->getJustifyItems()},
          frameInfo{frameInfo}, measured{measured}, mutate{mutate},
          childAvailableWidth{parentAvailableWidth},
          parentAvailableWidth{parentAvailableWidth}, parentAvailableHeight{parentAvailableHeight},
          minX{minX}, minY{minY}, maxX{maxX}, maxY{maxY}
    {}

    Constraints GridResolver::prepareChildConstraints(TreeNode* child) {
        auto preparedChildConstraints = childConstraints;

        preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
            child,
            childAvailableWidth,
            preparedChildConstraints.widthResolution
        );
        preparedChildConstraints.availableWidth = childAvailableWidth;
        preparedChildConstraints.inheritedProperties =
            parentConstraints.inheritedProperties;

        return preparedChildConstraints;
    }

    void GridResolver::phaseB() {
        auto& templateCols = node->getGridTemplateColumns();
        auto& templateRows = node->getGridTemplateRows();

        bool widthBasisIsIndefinite =
            parentConstraints.shrinkWidthToFit ||
            parentConstraints.widthResolution == AxisResolution::MinContent ||
            parentConstraints.widthResolution == AxisResolution::MaxContent ||
            (!measured.explicitWidth &&
             measured.explicitWidth.error() ==
                SizeResolveFailure::IndefiniteBasis);

        bool widthDefinite = !widthBasisIsIndefinite && !parentAvailableWidth.isAuto();
        auto widthBasis = widthDefinite ? parentAvailableWidth : Size::autoSize();

        float availableWidth = widthDefinite ? parentAvailableWidth.value : 0.0f;
        float colGap = node->getGridColumnGap().resolve(widthBasis).value_or(0.0f);

        for (size_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto childPos = childAsPtr->getPosition();
            if (childPos == Position::Absolute || childPos == Position::Fixed) 
                continue;

            Measured childMeasured = *childAsPtr->measured;
            auto preparedChildConstraints = prepareChildConstraints(childAsPtr);
            preparedChildConstraints.widthResolution = AxisResolution::MaxContent;
            preparedChildConstraints.intrinsicSizesAxis = Axis::Width;
            preparedChildConstraints.shrinkWidthToFit = true;
            childMeasured.explicitWidth = std::unexpected(SizeResolveFailure::Auto);
            if (childAsPtr->shared.aspectRatio)
                transferAspectRatio(
                    childMeasured.explicitWidth,
                    childMeasured.explicitHeight,
                    *childAsPtr->shared.aspectRatio
                );
            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childAsPtr, preparedChildConstraints.availableWidth, preparedChildConstraints.widthResolution, true);

            const auto& childOutput = tree.speculateLayout(
                frameInfo,
                childAsPtr,
                preparedChildConstraints,
                childMeasured
            );
            auto& childLayout = childOutput.layout;
            if (childLayout.outOfFlow) 
                continue;

            IntrinsicSizes intrinsicWidths = childOutput.intrinsicSizes.value_or(IntrinsicSizes{
                .minContent = Size::px(childLayout.computedBox.width),
                .maxContent = Size::px(childLayout.computedBox.width)
            });
            float minContent = intrinsicWidths.minContent.resolveOr(Size::autoSize());
            float maxContent = intrinsicWidths.maxContent.resolveOr(Size::autoSize());

            std::optional<float> preferredWidth;
            if (childLayout.resolvedSize.width) {
                preferredWidth = *childLayout.resolvedSize.width;
            } else if (childAsPtr->shared.width.isContentDependent()) {
                preferredWidth = resolveIntrinsicSize(childAsPtr->shared.width, intrinsicWidths, widthBasis);
            } else if (childAsPtr->shared.width.unit != style::Unit::Percent) {
                auto resolved = childAsPtr->shared.width.resolve(widthBasis);
                if (resolved) 
                    preferredWidth = *resolved;
            }

            std::optional<float> minWidth;
            if (childAsPtr->shared.minWidth.isContentDependent()) {
                minWidth = resolveIntrinsicSize(childAsPtr->shared.minWidth, intrinsicWidths, widthBasis);
            } else {
                auto resolved = childAsPtr->shared.minWidth.resolve(widthBasis);
                if (resolved) 
                    minWidth = *resolved;
            }

            std::optional<float> maxWidth;
            if (childAsPtr->shared.maxWidth.has_value()) {
                const auto& requestedMaxWidth = *childAsPtr->shared.maxWidth;
                if (requestedMaxWidth.isContentDependent()) {
                    maxWidth = resolveIntrinsicSize(requestedMaxWidth, intrinsicWidths, widthBasis);
                } else {
                    auto resolved = requestedMaxWidth.resolve(widthBasis);
                    if (resolved) 
                        maxWidth = *resolved;
                }
            }

            if (preferredWidth.has_value()) 
                minContent = maxContent = *preferredWidth;

            if (maxWidth.has_value()) {
                minContent = std::min(minContent, *maxWidth);
                maxContent = std::min(maxContent, *maxWidth);
            }
            if (minWidth.has_value()) {
                minContent = std::max(minContent, *minWidth);
                maxContent = std::max(maxContent, *minWidth);
            }

            float minimum = 0.0f;
            if (minWidth.has_value()) {
                minimum = *minWidth;
            } else if (preferredWidth.has_value() || childAsPtr->shared.overflow == Overflow::Visible) {
                minimum = minContent;
            }

            if (maxWidth.has_value()) 
                minimum = std::min(minimum, *maxWidth);

            gridLayout.addChild(i, childAsPtr, {.minimum = minimum, .minContent = minContent, .maxContent = maxContent});
        }

        gridLayout.resolveColumns(templateRows.size(), templateCols.size(), templateCols, availableWidth, colGap, widthDefinite);
        if (parentConstraints.intrinsicSizesAxis == Axis::Width) intrinsicSizes = gridLayout.columnIntrinsicSizes;
    }

    GridResolver::Bounds GridResolver::phaseC() {
        bool heightBasisIsIndefinite =
            parentConstraints.shrinkHeightToFit ||
            parentConstraints.heightResolution == AxisResolution::MinContent ||
            parentConstraints.heightResolution == AxisResolution::MaxContent ||
            (!measured.explicitHeight &&
             (measured.explicitHeight.error() == SizeResolveFailure::Auto ||
              measured.explicitHeight.error() == SizeResolveFailure::IndefiniteBasis));
        bool heightDefinite = !heightBasisIsIndefinite && !parentAvailableHeight.isAuto();
        Size heightBasis = heightDefinite ? parentAvailableHeight : Size::autoSize();
        float availableHeight = heightDefinite ? parentAvailableHeight.value : 0.0f;
        float rowGap = node->getGridRowGap().resolve(heightBasis).value_or(0.0f);

        for (auto& item : gridLayout.items) {
            auto childAsPtr = node->children[item.childIndex].get();
            auto& placement = item.placement;
            float cellX = gridLayout.colTracks[*placement.colStart].offset;
            float cellW = gridLayout.colTracks[*placement.colEnd - 1].offset + gridLayout.colTracks[*placement.colEnd - 1].size - cellX;
            float itemW = applyMinMax(cellW, childAsPtr->shared.minWidth, childAsPtr->shared.maxWidth, cellW);
            Measured childMeasured = *childAsPtr->measured;
            auto preparedChildConstraints = prepareChildConstraints(childAsPtr);
            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childAsPtr, Size::px(itemW), preparedChildConstraints.widthResolution);
            preparedChildConstraints.availableWidth = Size::px(itemW);
            preparedChildConstraints.availableHeight = Size::autoSize();
            preparedChildConstraints.shrinkWidthToFit = false;
            preparedChildConstraints.shrinkHeightToFit = false;
            preparedChildConstraints.heightResolution = AxisResolution::MaxContent;
            preparedChildConstraints.intrinsicSizesAxis = Axis::Height;
            childMeasured.explicitHeight = std::unexpected(SizeResolveFailure::Auto);

            JustifyItems effectiveJustify = justifyItems;
            auto selfJustify = childAsPtr->getJustifySelf();
            if (selfJustify != JustifySelf::Auto) {
                switch (selfJustify) {
                    case JustifySelf::Stretch: effectiveJustify = JustifyItems::Stretch; break;
                    case JustifySelf::Start:   effectiveJustify = JustifyItems::Start; break;
                    case JustifySelf::End:     effectiveJustify = JustifyItems::End; break;
                    case JustifySelf::Center:  effectiveJustify = JustifyItems::Center; break;
                    default: break;
                }
            }

            if (effectiveJustify == JustifyItems::Stretch && childAsPtr->shared.width.isAuto()) {
                childMeasured.explicitWidth = itemW;
            } else if (childAsPtr->shared.width.isAuto()) {
                preparedChildConstraints.shrinkWidthToFit = true;
            }
            if (childAsPtr->shared.height.isAuto())
                preparedChildConstraints.shrinkHeightToFit = true;

            if (childAsPtr->shared.aspectRatio)
                transferAspectRatio(
                    childMeasured.explicitWidth,
                    childMeasured.explicitHeight,
                    *childAsPtr->shared.aspectRatio
                );

            const auto& childOutput = tree.speculateLayout(frameInfo, childAsPtr, preparedChildConstraints, childMeasured);
            IntrinsicSizes intrinsicHeights = childOutput.intrinsicSizes.value_or(IntrinsicSizes{
                .minContent = Size::px(childOutput.layout.consumedHeight),
                .maxContent = Size::px(childOutput.layout.consumedHeight)
            });
            float minContent = intrinsicHeights.minContent.resolveOr(Size::autoSize());
            float maxContent = intrinsicHeights.maxContent.resolveOr(Size::autoSize());

            std::optional<float> preferredHeight;
            if (childOutput.layout.resolvedSize.height) {
                preferredHeight = *childOutput.layout.resolvedSize.height;
            } else if (childAsPtr->shared.height.isContentDependent()) {
                preferredHeight = resolveIntrinsicSize(childAsPtr->shared.height, intrinsicHeights, heightBasis);
            } else if (childAsPtr->shared.height.unit != style::Unit::Percent) {
                auto resolved = childAsPtr->shared.height.resolve(heightBasis);
                if (resolved)
                    preferredHeight = *resolved;
            }

            std::optional<float> minHeight;
            if (childAsPtr->shared.minHeight.isContentDependent()) {
                minHeight = resolveIntrinsicSize(childAsPtr->shared.minHeight, intrinsicHeights, heightBasis);
            } else {
                auto resolved = childAsPtr->shared.minHeight.resolve(heightBasis);
                if (resolved)
                    minHeight = *resolved;
            }

            std::optional<float> maxHeight;
            if (childAsPtr->shared.maxHeight.has_value()) {
                const auto& requestedMaxHeight = *childAsPtr->shared.maxHeight;
                if (requestedMaxHeight.isContentDependent()) {
                    maxHeight = resolveIntrinsicSize(requestedMaxHeight, intrinsicHeights, heightBasis);
                } else {
                    auto resolved = requestedMaxHeight.resolve(heightBasis);
                    if (resolved)
                        maxHeight = *resolved;
                }
            }

            if (preferredHeight.has_value())
                minContent = maxContent = *preferredHeight;
            if (maxHeight.has_value()) {
                minContent = std::min(minContent, *maxHeight);
                maxContent = std::min(maxContent, *maxHeight);
            }
            if (minHeight.has_value()) {
                minContent = std::max(minContent, *minHeight);
                maxContent = std::max(maxContent, *minHeight);
            }

            float minimum = 0.0f;
            if (minHeight.has_value()) {
                minimum = *minHeight;
            } else if (preferredHeight.has_value() || childAsPtr->shared.overflow == Overflow::Visible) {
                minimum = minContent;
            }
            if (maxHeight.has_value())
                minimum = std::min(minimum, *maxHeight);

            item.heightContributions = {.minimum = minimum, .minContent = minContent, .maxContent = maxContent};
        }

        gridLayout.resolveRows(node->getGridTemplateRows(), availableHeight, rowGap, heightDefinite);
        if (parentConstraints.intrinsicSizesAxis == Axis::Height)
            intrinsicSizes = gridLayout.rowIntrinsicSizes;

        for (auto& item : gridLayout.items) {
            auto childAsPtr = node->children[item.childIndex].get();
            auto& placement = item.placement;
            Measured childMeasured = *childAsPtr->measured;

            auto& colTracks = gridLayout.colTracks;
            auto& rowTracks = gridLayout.rowTracks;

            float cellX = colTracks[*placement.colStart].offset;
            float cellY = rowTracks[*placement.rowStart].offset;

            float cellW = colTracks[*placement.colEnd - 1].offset + colTracks[*placement.colEnd - 1].size - cellX;
            float cellH = rowTracks[*placement.rowEnd - 1].offset + rowTracks[*placement.rowEnd - 1].size - cellY;

            float itemW = applyMinMax(cellW, childAsPtr->shared.minWidth, childAsPtr->shared.maxWidth, cellW);
            float itemH = applyMinMax(cellH, childAsPtr->shared.minHeight, childAsPtr->shared.maxHeight, cellH);

            auto preparedChildConstraints =
                prepareChildConstraints(childAsPtr);

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(
                childAsPtr,
                Size::px(itemW),
                preparedChildConstraints.widthResolution
            );
            preparedChildConstraints.availableWidth = Size::px(itemW);
            preparedChildConstraints.availableHeight = Size::px(itemH);
            preparedChildConstraints.origin = {cellX, cellY};
            preparedChildConstraints.cursor = {cellX, cellY};
            preparedChildConstraints.shrinkWidthToFit = false;
            preparedChildConstraints.shrinkHeightToFit = false;

            // resolve alignment
            AlignItems effectiveAlign = alignItems;
            auto selfAlign = childAsPtr->getAlignSelf();
            if (selfAlign != AlignSelf::Auto) {
                switch (selfAlign) {
                    case AlignSelf::Stretch:   effectiveAlign = AlignItems::Stretch; break;
                    case AlignSelf::FlexStart: effectiveAlign = AlignItems::FlexStart; break;
                    case AlignSelf::FlexEnd:   effectiveAlign = AlignItems::FlexEnd; break;
                    case AlignSelf::Center:    effectiveAlign = AlignItems::Center; break;
                    default: break;
                }
            }

            JustifyItems effectiveJustify = justifyItems;
            auto selfJustify = childAsPtr->getJustifySelf();
            if (selfJustify != JustifySelf::Auto) {
                switch (selfJustify) {
                    case JustifySelf::Stretch: effectiveJustify = JustifyItems::Stretch; break;
                    case JustifySelf::Start:   effectiveJustify = JustifyItems::Start; break;
                    case JustifySelf::End:     effectiveJustify = JustifyItems::End; break;
                    case JustifySelf::Center:  effectiveJustify = JustifyItems::Center; break;
                    default: break;
                }
            }

            if (effectiveJustify == JustifyItems::Stretch &&
                childAsPtr->shared.width.isAuto()) {
                childMeasured.explicitWidth = itemW;
            } else if (childAsPtr->shared.width.isAuto()) {
                preparedChildConstraints.shrinkWidthToFit = true;
            }

            if (effectiveAlign == AlignItems::Stretch &&
                childAsPtr->shared.height.isAuto()) {
                childMeasured.explicitHeight = itemH;
            } else if (childAsPtr->shared.height.isAuto()) {
                preparedChildConstraints.shrinkHeightToFit = true;
            }

            if (childAsPtr->shared.aspectRatio)
                transferAspectRatio(
                    childMeasured.explicitWidth,
                    childMeasured.explicitHeight,
                    *childAsPtr->shared.aspectRatio
                );

            const LayoutOutput* childOutput = &tree.speculateLayout(frameInfo, childAsPtr, preparedChildConstraints, childMeasured);

            float dx = 0.0f;
            if (effectiveJustify == JustifyItems::Center) {
                dx = (cellW - childOutput->layout.computedBox.width) / 2.0f;
            } else if (effectiveJustify == JustifyItems::End) {
                dx = cellW - childOutput->layout.computedBox.width;
            }

            float dy = 0.0f;
            if (effectiveAlign == AlignItems::Center) {
                dy = (cellH - childOutput->layout.computedBox.height) / 2.0f;
            } else if (effectiveAlign == AlignItems::FlexEnd) {
                dy = cellH - childOutput->layout.computedBox.height;
            }

            preparedChildConstraints.origin.x += dx;
            preparedChildConstraints.origin.y += dy;
            preparedChildConstraints.cursor.x += dx;
            preparedChildConstraints.cursor.y += dy;

            std::optional<LayoutOutput> finalChildOutput;
            if (mutate) {
                finalChildOutput = tree.layoutPhase(
                    childAsPtr,
                    frameInfo,
                    preparedChildConstraints,
                    childMeasured
                );
                childOutput = &*finalChildOutput;
            } else if (dx != 0.0f || dy != 0.0f) {
                childOutput = &tree.speculateLayout(
                    frameInfo,
                    childAsPtr,
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
