#include "grid.hpp"
#include "new_sizing.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>
#include <variant>
#include <set>

namespace layout {
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

    // grid search funcs
    void Grid::mark(int row, int col) {
        occupied[row][col] = 1;
    }

    // there's definitely a more clever algo for this too lol
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

    // there's definitely a more clever algorithm for this lol
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


    // placement func 
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

    // sizing func
    /*
        complexity - the bitter lesson
        central sizing evaluator: still responsible for sizing of boxes (individual elements)
        however, track sizing exposes a fundementally different paradigm

        tracks are *unique to grid*, are outside of the box element model, and thus must be evaluated
        outside of sizing
        
        this invariably involes calling some sizing functions, and evaluating frs, etc..., but is fine

        tracks are outside the element sizing model; we will size tracks and then use their information
        to determine how children size themselves

        i.e. tracks become the sub-containers that children exist in

        we will commit to sizing tracks here but *NOT* elements

        elements will size themselves according to the constraints set by their track

        flex gets away without needing this bc it sizes the individual elements

        this is not sizing the elements, it sizing the tracks
    */

    auto generateSizeFunction(const SizeState& sizeState, const SizeState& available) -> SizeState {
        return std::visit(Overloaded{
            [](float resolved) -> SizeState {
                return resolved;
            },
            [&](const Size& size) -> SizeState {
                // valid sizing function types
                if (size.isFr() || size.isAuto() || size.isContentDependent()) {
                    return size;
                }

                // note: minmax + fit-content(val) not supported yet

                return calculateSize(size, available);
            },
            [&](const auto& other) -> SizeState {
                return calculateSize(other, available);
            }
        }, sizeState);
    };

    auto GridLayout::resolveTracks(std::vector<SizeState>& sizingFunctionReqs, const SizeState& available, float gap, bool isCol, IntrinsicSizes* intrinsicSizes) -> std::vector<Track> {
        /*
            this method sizes all tracks along a certain axis
            note: the track is the abstraction for a row/column, 
            in case that axis wording was confusing
        */
        

        // the sizing algorithm will utilize context from the grid reoslver
        // plus previously resolved sizes to determine how to size things

        // i think available should change to become the container size result?
        // thus we get available + the container's automatic min/max, which is useful for enabling
        // limited min/max content

        // phase 1: determine sizing functions
        auto numTracks = sizingFunctionReqs.size();
        
        std::vector<SizeState> minSizingFunctions {};
        std::vector<SizeState> maxSizingFunctions {};

        for (auto& sizingFunctionReq : sizingFunctionReqs) {
            auto sizingFunction = generateSizeFunction(sizingFunctionReq, available);
            
            std::visit(Overloaded{
                [&](float resolved){
                    minSizingFunctions.push_back(resolved);
                    maxSizingFunctions.push_back(resolved);
                },
                [&](Size& size){
                    if (size.isAuto() || size.isContentDependent()) {
                        minSizingFunctions.push_back(size);
                        maxSizingFunctions.push_back(size);
                    }else if (size.isFr()) {
                        minSizingFunctions.push_back(Size::autoSize());
                        maxSizingFunctions.push_back(size);
                    }else {
                        // unrepresentible case? shore this up
                        // sensible default preferred over error
                        // maybe auto? what is the grid default behavior?
                        // what if the track sizing needed more info? is that plausible 
                        // (i.e. like available representation)
                    }   
                },
                [](auto&) {
                    // unrepresentible case? shore this up
                }
            }, sizingFunction);
        }
       
        // phase 2: initialize track state
        // depends on the min sizing function + max sizing func
        std::vector<float> baseSizes;

        for (const auto& minSizingFunction : minSizingFunctions) {
            std::visit(Overloaded{
                [&](float resolved){
                    baseSizes.push_back(resolved);
                    baseSizes.push_back(resolved);
                },
                [&](Size& size){
                    // default to 0 for intrinsic funcs
                    if (size.isAuto() || size.isContentDependent()) {
                        baseSizes.push_back(0.0f);
                        baseSizes.push_back(0.0f);;
                    }else if (size.isFr()) {
                        // not in spec lol?
                    }else {
                        // unrepresentible case? shore this up
                        // sensible default preferred over error
                        // maybe auto? what is the grid default behavior?
                        // what if the track sizing needed more info? is that plausible 
                        // (i.e. like available representation)
                    }   
                },
                [](auto&) {
                    // unrepresentible case? shore this up
                }
            }, minSizingFunction);
        }

        std::vector<float> growthLimits;
        for (const auto& maxSizingFunction : maxSizingFunctions) {
            std::visit(Overloaded{
                [&](float resolved){
                    growthLimits.push_back(resolved);
                    growthLimits.push_back(resolved);
                },
                [&](Size& size){
                    // i think all of these default to infinity? (intrinsicMin,Max,Fit,Auto + fr)
                    growthLimits.push_back(std::numeric_limits<float>::infinity());
                },
                [](auto&) {
                    // unrepresentible case? shore this up
                }
            }, maxSizingFunction);
        }
 
        // next: spec says "shim baseline items" - i dont think I care; I have no clue what this even means

        /*
            The Loop: the basic primitive of grid
            
            imagine this 2d grid
            |||
            v|v
            |v|
            |||
            vvv

            each | v denotes a spot on the grid
            Items create a sequence of |'s ended with a v (| start/mid of a span, v is the ned)

            we're going to loop over items and track their starting | and v (start and end)
            we're going to check if that span fits our current sizing op
            then we're process as necessary
        */

        // next: process items taking up 1 non-fr track
        for (auto i = 0; i < this->items.size(); ++i) {
            auto& item = items[i];

            // y is this type optional; fix colstart/rowstart optionaltiy (llm's job)
            uint32_t start = isCol ? *item.placement.colStart : *item.placement.rowStart; // hate this selection method
            uint32_t end = isCol ? *item.placement.colEnd : *item.placement.rowEnd;
            uint32_t span = end - start;

            // skip; we're only process 1 span, non-fr tracks now
            // here, start = end, so it doesn't matter what we index
            if (span != 1) {
                continue;
            }

            auto minSizingFunction = minSizingFunctions[start];

            // i hope these are corrected in addChild to use the size result
            auto minContent = isCol ? item.widthContributions.minContent : item.heightContributions.minContent;
            auto maxContent = isCol ? item.widthContributions.maxContent : item.heightContributions.maxContent;

            // we only adjust base size according to our intrinsic tracks
            std::visit(Overloaded{
                [&](Size& size){
                    if (size.isMinContent()) {
                        baseSizes[start] = std::max(baseSizes[start], minContent);          
                    }else if (size.isMaxContent()) {
                        baseSizes[start] = std::max(baseSizes[start], maxContent);
                    }else if (size.isFitContent()) {
                        // not fully supported yet?
                    }else if (size.isAuto()) {
                        // odd ball case
                        // if grid container is min/max content:
                        // use either:
                        // std::max(baseSizes[start], minContent); (min)
                        // or std::max(baseSizes[start], maxContent); (max)
                        // clamped by max track sizing case. very very fucking weird

                        // else, just use (only this case is supported rn)
                        // std::max(baseSizes[start], minContent) (min)
                        baseSizes[start] = std::max(baseSizes[start], minContent);
                    }
                },
                [&](auto&) {}
            }, minSizingFunction);

            // ok; grid template columns DEFO should not be a size?
            // actually it should be, but i dont think we have enough separation
            // let me finish wiritng this algo, but we're basically redoing sizing
            // which annoys me. hm


            auto maxSizingFunction = maxSizingFunctions[start];

            // we only adjust base size according to our intrinsic tracks
            std::visit(Overloaded{
                [&](Size& size){
                    if (size.isMinContent()) {
                        growthLimits[start] = std::max(growthLimits[start], minContent);          
                    }else if (size.isMaxContent()) {
                        growthLimits[start] = std::max(growthLimits[start], maxContent);
                    }else if (size.isFitContent()) {
                        // not yet impl yet
                        // growthLimits[start] = std::max(baseSizes[start], maxContent);
                        // clamp by fit content arg? what is the *fit_content* arg
                    }
                },
                [&](auto&) {}
            }, maxSizingFunction);


            // correct growth limits in case they're less than start
            growthLimits[start] = std::max(growthLimits[start], baseSizes[start]);
        }

        // process items spamming at least 2 non-fr tracks in increasing order
        // spec calls this step: "Increase sizes to accommodate spanning items crossing content-sized tracks"

        /*
            Note: For items with a specified minimum size of auto (the initial value), 
            the minimum contribution is usually equivalent to the min-content contribution—​but 
            can differ in some cases, see § 6.6 Automatic Minimum Size of Grid Items. 
            Also, minimum contribution ≤ min-content contribution ≤ max-content contribution.
        */

        // then, lets acc do the loop thingy?
        for (uint32_t targetSpan = 2; targetSpan < numTracks; ++targetSpan) {
            float totalBaseSizeExtraSpace = 0.0f;
            float totalGrowthLimitExtraSpace = 0.0f;
            std::set<int> affectedTracks {};

            // loop over items as we have it
            // consider the singular track items
            // extra space: sum of size contribution - track size
            // track size will just be the base size
            // but we will need to collect extra spaces for minimum, min-content, max content

            // we will also need to do it for growth limits
            // we will track extra space for max-content and min-content contributions - growth limit

            // if the contaienr is under a min or max constraint,
            // replace max-content/min-content with  limited max-content contributions.
            // see above for that change

            /*
                "Mark any tracks whose growth limit changed from infinite to finite in this step as infinitely growable for the next step."
                new vector emerges
            */


            for (auto i = 0; i < this->items.size(); ++i) { 
                auto& item = items[i];

                // why is this type optional; fix colstart/rowstart optionaltiy (llm's job)
                uint32_t start = isCol ? *item.placement.colStart : *item.placement.rowStart; // hate this selection method
                uint32_t end = isCol ? *item.placement.colEnd : *item.placement.rowEnd;
                uint32_t span = end - start;

                if (span != targetSpan) {
                    continue;
                }

                // insert affected tracks
                affectedTracks.insert_range(std::views::iota(start, end + 1));

                // gather extra space contributions
                auto minSizingFunction = minSizingFunctions[start];
                auto maxSizingFunction = maxSizingFunctions[start];

                // i hope these are corrected in addChild to use the size result
                auto minContribution = isCol ? item.widthContributions.minimum : item.heightContributions.minimum;
                auto minContent = isCol ? item.widthContributions.minContent : item.heightContributions.minContent;
                auto maxContent = isCol ? item.widthContributions.maxContent : item.heightContributions.maxContent;

                // base size extra space
                // todo (requires new arg, probably a size result)
                // adjust for being under min/max constraints
                
                // base size formula: std::max(0, contribution - sum of track sizes)
                // i approximate this as std::max(baseSizeExtraSpace, contribution - sum of track sizes)
                // bc it starts as 0.0f and folds nicely in parallel anyways

                float spanBaseSize = std::ranges::fold_left(std::span(baseSizes).subspan(start, end), 0, std::plus{});
                float baseSizeExtraSpace = 0.0f;
                // 1. for intrinsic minimums
                baseSizeExtraSpace = std::max(baseSizeExtraSpace, minContribution - spanBaseSize);

                // 2. for content based minimums
                baseSizeExtraSpace = std::max(baseSizeExtraSpace, minContent - spanBaseSize);

                // 3. for max-content minimums
                baseSizeExtraSpace = std::max(baseSizeExtraSpace, maxContent - spanBaseSize);

                totalBaseSizeExtraSpace += baseSizeExtraSpace;

                float spanGrowthLimitSpace = std::ranges::fold_left(std::span(growthLimits).subspan(start, end), 0, std::plus{});
                float growthExtraSpace = 0.0f;
                // 4. For intrinsic maximums: expand growth limits
                growthExtraSpace = std::max(growthExtraSpace, minContribution - spanGrowthLimitSpace);

                totalGrowthLimitExtraSpace += growthExtraSpace;
                
                // mark infinitely growable? I dont really understand their explanation as to *why*
                
            }

            // distribute extra space to tracks?

            for (auto& affectedTrack : affectedTracks) {
                baseSizes[affectedTrack] += totalBaseSizeExtraSpace / affectedTracks.size();
                growthLimits[affectedTrack] += totalGrowthLimitExtraSpace / affectedTracks.size();

                // correct growth limits
                growthLimits[affectedTrack] = std::max(growthLimits[affectedTrack], baseSizes[affectedTrack]);
            }

        }

        // 4 is the previous step repeated for flexible tracks
        
        // at the end, just set all growth limits with infinite size to base size
    }

    void GridLayout::resolveColumns(size_t numRows, size_t numCols, const std::vector<Size>& templateCols, const SizeState& availableWidth, float colGap) {
        resolveStructure(numRows, numCols);
        std::vector<SizeState> colDefs(grid.numCols, Size::autoSize());

        for (int j = 0; j < templateCols.size(); ++j)
            colDefs[j] = templateCols[j];

        // colTracks = resolveTracks(colDefs, availableWidth, colGap, true, &columnIntrinsicSizes);
    }

    void GridLayout::resolveRows(const std::vector<Size>& templateRows, const SizeState& availableHeight, float rowGap) {
        std::vector<SizeState> rowDefs(grid.numRows, Size::autoSize());
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

    // resolve cols fully
    void GridResolver::phaseB() {
        // idt these should be sizes?
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

    // resolve rows fully
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

            // wtf is this lol
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
