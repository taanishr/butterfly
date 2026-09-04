#include "grid.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include <algorithm>
#include <optional>
#include <variant>

namespace layout {
    namespace {
        void distributeSpanningContribution(std::vector<float>& contributions, const std::vector<float>& fixedSizes, const std::vector<bool>& fixedTracks, const std::vector<Size>& definitions, size_t start, size_t end, float gap, float contribution) {
            float covered = gap * static_cast<float>(end - start - 1);
            bool spansFractionTrack = false;
            for (size_t i = start; i < end; ++i) {
                covered += fixedTracks[i] ? fixedSizes[i] : contributions[i];
                spansFractionTrack = spansFractionTrack || definitions[i].isFr(); // why isFr()?
            }

            float extra = contribution - covered;
            if (extra <= 0.0f)
                return;

            float totalWeight = 0.0f;
            size_t eligibleTracks = 0;
            for (size_t i = start; i < end; ++i) {
                if (fixedTracks[i] || (spansFractionTrack && !definitions[i].isFr())) // again, why isFr()??? tf lol
                    continue;
                totalWeight += spansFractionTrack ? definitions[i].value : 1.0f;
                eligibleTracks++;
            }
            if (eligibleTracks == 0)
                return;

            for (size_t i = start; i < end; ++i) {
                if (fixedTracks[i] || (spansFractionTrack && !definitions[i].isFr())) // sigh, more fr
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
        for (int r = row; r < row + spanRows; ++r) {
            for (int c = col; c < col + spanCols; ++c) {
                if (occupied[r][c]) {
                    return false;
                }
            }
        }
    
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
                for (auto& row : occupied) {
                    row.push_back(0);
                }
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
                for (int r = *placement.rowStart; r < *placement.rowEnd; ++r) {
                    for (int c = *placement.colStart; c < *placement.colEnd; ++c) {
                        grid.mark(r, c);
                    }
                }
            }
        }

        // place items with unresolved placements
        for (auto& item : items) {
            auto& placement = item.placement;
            if (!placement.colNeedsResolution() && !placement.rowNeedsResolution()) 
                continue;

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


    std::vector<Track> GridLayout::resolveTracks(std::vector<Size>& defs, const SizeState& available, float gap, bool isCol, IntrinsicSizes* intrinsicSizes) {
        size_t n = defs.size();
        float totalGap = (n > 1) ? gap * (float)(n - 1) : 0;
        const float* resolvedAvailable = std::get_if<float>(&available);
        float usable = resolvedAvailable ? std::max(0.0f, *resolvedAvailable - totalGap) : 0.0f;
        Size basis = resolvedAvailable ? Size::px(*resolvedAvailable) : Size::autoSize();

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
            intrinsicSizes->minimum = intrinsicMin;
            intrinsicSizes->maximum = intrinsicMax;
        }

        std::vector<float> sizes(n, 0.0f);
        float nonFractionTotal = 0.0f;
        float fractionTotal = 0.0f;
        for (size_t i = 0; i < n; ++i) {
            // why the fuck does this exist; why are we siwtching between 12 units
            if (fixedTracks[i]) {
                sizes[i] = fixedSizes[i];
            } else if (defs[i].unit == style::Unit::MinContent) {
                sizes[i] = minContents[i];
            } else if (defs[i].unit == style::Unit::MaxContent) {
                sizes[i] = maxContents[i];
            } else if (defs[i].unit == style::Unit::FitContent) {
                IntrinsicSizes contributions{.minimum = minContents[i], .maximum = maxContents[i]};

                // what the fuck is... bro what am i reading. idek how to repalce htis
                // sizes[i] = resolveIntrinsicSize(defs[i], contributions, basis);
            } else if (defs[i].isFr() && resolvedAvailable) {
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

    void GridLayout::resolveColumns(size_t numRows, size_t numCols, const std::vector<Size>& templateCols, const SizeState& availableWidth, float colGap) {
        resolveStructure(numRows, numCols);
        std::vector<Size> colDefs(grid.numCols, Size::autoSize());

        for (int j = 0; j < templateCols.size(); ++j)
            colDefs[j] = templateCols[j];

        colTracks = resolveTracks(colDefs, availableWidth, colGap, true, &columnIntrinsicSizes);
    }

    void GridLayout::resolveRows(const std::vector<Size>& templateRows, const SizeState& availableHeight, float rowGap) {
        std::vector<Size> rowDefs(grid.numRows, Size::autoSize());
        for (int i = 0; i < templateRows.size(); ++i)
            rowDefs[i] = templateRows[i];

        rowTracks = resolveTracks(rowDefs, availableHeight, rowGap, false, &rowIntrinsicSizes);
    }

    GridResolver::GridResolver(RenderTree& tree, TreeNode* node,
                               const Constraints& parentConstraints,
                               const Constraints& childConstraints,
                               const FrameInfo& frameInfo,
                               const SizePair& availableSize, bool mutate,
                               std::unordered_map<size_t, SizeResult>& sizeCache,
                               float minX, float minY, float maxX, float maxY)
        : tree{tree}, node{node}, parentConstraints{parentConstraints},
          childConstraints{childConstraints},
          alignItems{node->getAlignItems()},
          justifyItems{node->getJustifyItems()},
          frameInfo{frameInfo}, availableSize{availableSize}, mutate{mutate},
          sizeCache{sizeCache},
          minX{minX}, minY{minY}, maxX{maxX}, maxY{maxY}
    {}

    Constraints GridResolver::prepareChildConstraints() {
        auto preparedChildConstraints = childConstraints;
        preparedChildConstraints.inheritedProperties = parentConstraints.inheritedProperties;

        return preparedChildConstraints;
    }

    void GridResolver::phaseB() {
        auto& templateCols = node->getGridTemplateColumns();
        auto& templateRows = node->getGridTemplateRows();

        float colGap = std::visit(Overloaded {
            [&](float width) { return node->getGridColumnGap().resolve(Size::px(width)).value_or(0.0f); },
            [&](const auto&) { return node->getGridColumnGap().resolve(Size::autoSize()).value_or(0.0f); },
        }, availableSize.width);

        for (size_t i = 0; i < node->children.size(); ++i) {
            auto childAsPtr = node->children[i].get();
            auto childPos = childAsPtr->getPosition();
            if (childPos == Position::Absolute || childPos == Position::Fixed) 
                continue;

            Measured childMeasured = *childAsPtr->measured;
            auto preparedChildConstraints = prepareChildConstraints();
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
                .automaticMinimumWidth = childAsPtr->shared.overflow == Overflow::Scroll
                    ? AutomaticMinimum::Zero
                    : AutomaticMinimum::ContentBased,
                .automaticMinimumHeight = AutomaticMinimum::Zero,
                .intrinsicWidthRequest = IntrinsicRequest::Both,
                .tag = "grid phase B, column contributions"
            };

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childAsPtr, {
                .availableWidth = childRequest.available.width,
                .widthRequest = childRequest.intrinsicWidthRequest,
                .trackIntrinsicWidth = false,
            });

            SizeResult childSizing = evaluateSize(
                tree, childAsPtr, frameInfo, preparedChildConstraints,
                childMeasured, childRequest, sizeCache
            );

            const auto& intrinsicWidths = *childSizing.widthIntrinsicSizes;
            float minContent = std::get<float>(intrinsicWidths.minimum);
            float maxContent = std::get<float>(intrinsicWidths.maximum);
            const float* preferredWidth = std::get_if<float>(&childSizing.outerSize.width);
            const float* minWidth = std::get_if<float>(&childSizing.minimum.width);
            const float* maxWidth = std::get_if<float>(&childSizing.maximum.width);

            if (preferredWidth)
                minContent = maxContent = *preferredWidth;

            if (maxWidth) {
                minContent = std::min(minContent, *maxWidth);
                maxContent = std::min(maxContent, *maxWidth);
            }
            if (minWidth) {
                minContent = std::max(minContent, *minWidth);
                maxContent = std::max(maxContent, *minWidth);
            }

            float minimum = std::get<float>(childSizing.minimum.width);

            if (maxWidth)
                minimum = std::min(minimum, *maxWidth);

            gridLayout.addChild(i, childAsPtr, {.minimum = minimum, .minContent = minContent, .maxContent = maxContent});
        }

        gridLayout.resolveColumns(templateRows.size(), templateCols.size(), templateCols, availableSize.width, colGap);
    }

    GridResolver::Bounds GridResolver::phaseC() {
        float rowGap = std::visit(Overloaded {
            [&](float height) { return node->getGridRowGap().resolve(Size::px(height)).value_or(0.0f); },
            [&](const auto&) { return node->getGridRowGap().resolve(Size::autoSize()).value_or(0.0f); },
        }, availableSize.height);

        for (auto& item : gridLayout.items) {
            auto childAsPtr = node->children[item.childIndex].get();
            auto& placement = item.placement;
            float cellX = gridLayout.colTracks[*placement.colStart].offset;
            float cellW = gridLayout.colTracks[*placement.colEnd - 1].offset + gridLayout.colTracks[*placement.colEnd - 1].size - cellX;
            Measured childMeasured = *childAsPtr->measured;
            auto preparedChildConstraints = prepareChildConstraints();

            JustifyItems effectiveJustify = justifyItems;
            auto selfJustify = childAsPtr->getJustifySelf();
            if (selfJustify != JustifySelf::Auto) {
                switch (selfJustify) {
                    case JustifySelf::Stretch: {
                        effectiveJustify = JustifyItems::Stretch; 
                        break;
                    }
                    case JustifySelf::Start: {
                        effectiveJustify = JustifyItems::Start; 
                        break;
                    }
                    case JustifySelf::End: {
                        effectiveJustify = JustifyItems::End; 
                        break;
                    }
                    case JustifySelf::Center:  {
                        effectiveJustify = JustifyItems::Center; 
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }

            SizePair childAvailableSize = availableSize;
            childAvailableSize.width = cellW;

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
                .available = childAvailableSize,
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
                .automaticWidth = effectiveJustify == JustifyItems::Stretch
                    ? AutomaticSizing::UseAvailable
                    : AutomaticSizing::UseContent,
                .automaticHeight = AutomaticSizing::UseContent,
                .automaticMinimumWidth = AutomaticMinimum::Zero,
                .automaticMinimumHeight = childAsPtr->shared.overflow == Overflow::Scroll
                    ? AutomaticMinimum::Zero
                    : AutomaticMinimum::ContentBased,
                .intrinsicHeightRequest = IntrinsicRequest::Both,
                .tag = "grid phase C, row contributions"
            };

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childAsPtr, {
                .availableWidth = childRequest.available.width,
                .widthRequest = childRequest.intrinsicWidthRequest,
                .trackIntrinsicWidth = false,
            });

            LayoutResult childOutput = tree.layoutRecursive(
                childAsPtr, frameInfo, preparedChildConstraints,
                childMeasured, false, childRequest
            );
            const SizeResult& childSizing = childOutput.sizeResult;

            const auto& intrinsicHeights = *childSizing.heightIntrinsicSizes;
            float minContent = std::get<float>(intrinsicHeights.minimum);
            float maxContent = std::get<float>(intrinsicHeights.maximum);
            const float* preferredHeight = std::get_if<float>(&childSizing.outerSize.height);
            const float* minHeight = std::get_if<float>(&childSizing.minimum.height);
            const float* maxHeight = std::get_if<float>(&childSizing.maximum.height);

            if (preferredHeight)
                minContent = maxContent = *preferredHeight;
            if (maxHeight) {
                minContent = std::min(minContent, *maxHeight);
                maxContent = std::min(maxContent, *maxHeight);
            }
            if (minHeight) {
                minContent = std::max(minContent, *minHeight);
                maxContent = std::max(maxContent, *minHeight);
            }

            float minimum = std::get<float>(childSizing.minimum.height);
            if (maxHeight)
                minimum = std::min(minimum, *maxHeight);

            item.heightContributions = {.minimum = minimum, .minContent = minContent, .maxContent = maxContent};
        }

        gridLayout.resolveRows(node->getGridTemplateRows(), availableSize.height, rowGap);

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

            auto preparedChildConstraints = prepareChildConstraints();
            preparedChildConstraints.origin = {cellX, cellY};
            preparedChildConstraints.cursor = {cellX, cellY};

            // resolve alignment
            AlignItems effectiveAlign = alignItems;
            auto selfAlign = childAsPtr->getAlignSelf();
            if (selfAlign != AlignSelf::Auto) {
                switch (selfAlign) {
                    case AlignSelf::Stretch: {
                        effectiveAlign = AlignItems::Stretch; 
                        break;
                    }
                    case AlignSelf::FlexStart: {
                        effectiveAlign = AlignItems::FlexStart; 
                        break;
                    }
                    case AlignSelf::FlexEnd: {
                        effectiveAlign = AlignItems::FlexEnd; 
                        break;
                    }
                    case AlignSelf::Center: {
                        effectiveAlign = AlignItems::Center; 
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }

            JustifyItems effectiveJustify = justifyItems;
            auto selfJustify = childAsPtr->getJustifySelf();
            if (selfJustify != JustifySelf::Auto) {
                switch (selfJustify) {
                    case JustifySelf::Stretch: {
                        effectiveJustify = JustifyItems::Stretch; 
                        break;
                    }
                    case JustifySelf::Start: {
                        effectiveJustify = JustifyItems::Start; 
                        break;
                    }
                    case JustifySelf::End: {
                        effectiveJustify = JustifyItems::End; 
                        break;
                    }
                    case JustifySelf::Center: {
                        effectiveJustify = JustifyItems::Center; 
                        break;
                    }
                    default: {
                        break;
                    }
                }
            }

            SizePair childAvailableSize {
                .width = cellW,
                .height = cellH,
            };
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
                .available = childAvailableSize,
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
                .automaticWidth = effectiveJustify == JustifyItems::Stretch
                    ? AutomaticSizing::UseAvailable
                    : AutomaticSizing::UseContent,
                .automaticHeight = effectiveAlign == AlignItems::Stretch
                    ? AutomaticSizing::UseAvailable
                    : AutomaticSizing::UseContent,
                .automaticMinimumWidth = AutomaticMinimum::Zero,
                .automaticMinimumHeight = AutomaticMinimum::Zero,
                .tag = "grid phase C, final request"
            };

            preparedChildConstraints.inlineFormatting = buildIsolatedInlineBoxes(childAsPtr, {
                .availableWidth = childRequest.available.width,
                .widthRequest = childRequest.intrinsicWidthRequest,
                .trackIntrinsicWidth = false,
            });

            LayoutResult childOutput = tree.layoutRecursive(
                childAsPtr, frameInfo, preparedChildConstraints,
                childMeasured, false, childRequest
            );

            float dx = 0.0f;
            if (effectiveJustify == JustifyItems::Center) {
                dx = (cellW - childOutput.layout.computedBox.width) / 2.0f;
            } else if (effectiveJustify == JustifyItems::End) {
                dx = cellW - childOutput.layout.computedBox.width;
            }

            float dy = 0.0f;
            if (effectiveAlign == AlignItems::Center) {
                dy = (cellH - childOutput.layout.computedBox.height) / 2.0f;
            } else if (effectiveAlign == AlignItems::FlexEnd) {
                dy = cellH - childOutput.layout.computedBox.height;
            }

            preparedChildConstraints.origin.x += dx;
            preparedChildConstraints.origin.y += dy;
            preparedChildConstraints.cursor.x += dx;
            preparedChildConstraints.cursor.y += dy;

            // interesting? why not just pass... mutate?
            if (mutate) {
                childOutput = tree.layoutRecursive(
                    childAsPtr, frameInfo, preparedChildConstraints,
                    childMeasured, true, childRequest
                );
            } else if (dx != 0.0f || dy != 0.0f) {
                childOutput = tree.layoutRecursive(
                    childAsPtr, frameInfo, preparedChildConstraints,
                    childMeasured, false, childRequest
                );
            }

            const auto& childLayout = childOutput.layout;

            maxX = std::max(maxX, childLayout.computedBox.x + childLayout.computedBox.width);
            maxY = std::max(maxY, childLayout.computedBox.y + childLayout.computedBox.height);
        }

        return {maxX, maxY};
    }
}
