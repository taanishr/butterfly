#include "grid.hpp"
#include "new_sizing.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <map>
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

    auto GridLayout::resolveTracks(std::vector<SizeState>& sizingFunctionReqs, const SizeResult& containerSize, float gap, bool isCol, IntrinsicSizes* intrinsicSizes) -> std::vector<Track> {
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

        const auto& available = isCol ? containerSize.innerSize.width : containerSize.innerSize.height;
        bool containerHasMinContentConstraint = false;
        if (std::holds_alternative<Size>(available)) {
            containerHasMinContentConstraint = std::get<Size>(available).isMinContent();
        }

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
                },
                [&](Size& size){
                    // default to 0 for intrinsic funcs
                    if (size.isAuto() || size.isContentDependent()) {
                        baseSizes.push_back(0.0f);
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
 
        // next: "shim baseline items"
        // skipped for now in this impl; applied later

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
                        if (containerHasMinContentConstraint) {
                            float limitedMinContent = minContent;
                            if (std::holds_alternative<float>(maxSizingFunctions[start])) {
                                float fixedTrackMaximum = std::get<float>(maxSizingFunctions[start]);
                                limitedMinContent = std::min(limitedMinContent, fixedTrackMaximum);
                            }
                            limitedMinContent = std::max(limitedMinContent, item.widthContributions.minimum);
                            baseSizes[start] = std::max(baseSizes[start], limitedMinContent);
                        }else {
                            // else, just use
                            // std::max(baseSizes[start], minContent) (min)
                            baseSizes[start] = std::max(baseSizes[start], minContent);
                        }
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
        constexpr int intrinsicMinimums = 0;
        constexpr int contentBasedMinimums = 1;
        constexpr int maxContentMinimums = 2;
        constexpr int intrinsicMaximums = 3;
        constexpr int maxContentMaximums = 4;
        constexpr std::array passes = {intrinsicMinimums, contentBasedMinimums, maxContentMinimums, intrinsicMaximums, maxContentMaximums};

        for (uint32_t targetSpan = 2; targetSpan < numTracks; ++targetSpan) {
            // loop over items as we have it
            // extra space: sum of size contribution - track size
            // track size will just be the base size
            // but we will need to collect extra spaces for minimum, min-content, max content

            // we will also need to do it for growth limits
            // we will track extra space for max-content and min-content contributions - growth limit

            // if the contaienr is under a min or max constraint,
            // replace max-content/min-content with  limited max-content contributions.
            // see above for that change


            // confusing wording: this is per item? not per track
            
            /*
                "Mark any tracks whose growth limit changed from infinite to finite in this step as infinitely growable for the next step."
                new vector emerges
            */

            // while *extra space exists*
            // keep distributing
            for (auto pass : passes) {
                std::map<int, float> baseSizePlannedIncreases {};
                std::map<int, float> growthLimitPlannedIncreases {};

                for (auto i = 0; i < this->items.size(); ++i) {
                    auto& item = items[i];

                    // why is this type optional; fix colstart/rowstart optionaltiy (llm's job)
                    uint32_t start = isCol ? *item.placement.colStart : *item.placement.rowStart; // hate this selection method
                    uint32_t end = isCol ? *item.placement.colEnd : *item.placement.rowEnd;
                    uint32_t span = end - start;

                    if (span != targetSpan) {
                        continue;
                    }

                    // init affected tracks
                    std::set<int> affectedTracks {};
                    
                    // gather extra space contributions

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
                    float spanGrowthLimitSpace = std::ranges::fold_left(std::span(growthLimits).subspan(start, end), 0, std::plus{});
                    float baseSizeExtraSpace = 0.0f;
                    float growthExtraSpace = 0.0f;

                    bool spansFlexibleTrack = std::ranges::any_of(
                                         std::span{maxSizingFunctions.begin() + start, end - start},
                                          [](auto& f){ 
                                                    return std::holds_alternative<Size>(f) && std::get<Size>(f).isFr();
                                                }
                                            );

                    if (spansFlexibleTrack) {
                        continue;
                    }
                    
                    for (auto spannedTrack : std::views::iota(start, end )) {
                        // only count tracks s.t. they're counted in pass?
                        // so maybe make this spannedTracks
                        // vs. affectedTracks
                        // then add to the affectedTracks set in the visitor
                        // and hten divide by that after

                        auto minSizingFunction = minSizingFunctions[spannedTrack];
                        auto maxSizingFunction = maxSizingFunctions[spannedTrack];
                     
                        std::visit(Overloaded{
                            [&](Size& size){
                                if (pass == intrinsicMinimums && (size.isAuto() || size.isContentDependent())) {
                                    // 1. for intrinsic minimums
                                    float intrinsicContribution = 0.0f;
                                    if (containerHasMinContentConstraint) {
                                        float limitedMinContent = minContent;
                                        if (std::holds_alternative<float>(maxSizingFunctions[start])) {
                                            float fixedTrackMaximum = std::get<float>(maxSizingFunctions[start]);
                                            limitedMinContent = std::min(limitedMinContent, fixedTrackMaximum);
                                        }
                                        limitedMinContent = std::max(limitedMinContent, item.widthContributions.minimum);
                                        intrinsicContribution = minContribution;
                                    }else {
                                        intrinsicContribution = minContribution;
                                    }

                                    baseSizeExtraSpace = std::max(baseSizeExtraSpace, intrinsicContribution - spanBaseSize);
                                    affectedTracks.insert(spannedTrack);
                                }else if(pass == contentBasedMinimums && (size.isMinContent() || size.isMaxContent())) {
                                    // 2. for content based minimums
                                    baseSizeExtraSpace = std::max(baseSizeExtraSpace, minContent - spanBaseSize);
                                    affectedTracks.insert(spannedTrack);
                                }else if (pass == maxContentMinimums && size.isMaxContent()) {
                                    // 3. for max-content minimums
                                    baseSizeExtraSpace = std::max(baseSizeExtraSpace, maxContent - spanBaseSize);
                                    affectedTracks.insert(spannedTrack);
                                }
                            },
                            [&](auto&) {
                                // dont alter intrinsic growth of base sizes/growth limits
                            }
                        }, minSizingFunction);

                        std::visit(Overloaded{
                            [&](Size& size){
                                // intrinsic; according to the grid algo, is:
                                // isAuto() || isContentDependent()
                                if (pass == intrinsicMaximums && (size.isAuto() || size.isContentDependent())) {
                                    growthExtraSpace = std::max(growthExtraSpace, minContent - spanGrowthLimitSpace);
                                    affectedTracks.insert(spannedTrack);
                                }else if (pass == maxContentMaximums && (size.isMaxContent() || size.isFitContent())) {
                                    growthExtraSpace = std::max(growthExtraSpace, maxContent - spanGrowthLimitSpace);
                                    affectedTracks.insert(spannedTrack);
                                }
                            },
                            [&](auto&) {
                                // dont alter intrinsic growth of base sizes/growth limits
                            }
                        }, maxSizingFunction);

                        // baseSizePlannedIncreases[affectedTrack] = std::max(baseSizePlannedIncreases[affectedTrack], baseSizeExtraSpace / affectedTracks.size());
                        // growthLimitPlannedIncreases[affectedTrack] = std::max(growthLimitPlannedIncreases[affectedTrack], growthExtraSpace / affectedTracks.size());
                    }

                    for (auto affectedTrack : affectedTracks) {
                        baseSizePlannedIncreases[affectedTrack] = std::max(baseSizePlannedIncreases[affectedTrack], baseSizeExtraSpace / affectedTracks.size());
                        growthLimitPlannedIncreases[affectedTrack] = std::max(growthLimitPlannedIncreases[affectedTrack], growthExtraSpace / affectedTracks.size());
                    }
                    
                
                    // mark infinitely growable? I dont really understand their explanation as to *why*
                
                }

                // distribute extra space to tracks? does this need to move, idrk or think so

                for (auto& [track, plannedIncrease] : baseSizePlannedIncreases) {
                    baseSizes[track] += plannedIncrease;
                }

                for (auto& [track, plannedIncrease] : growthLimitPlannedIncreases) {
                    // how does something go from infinite to... finite?
                    if (growthLimits[track] != std::numeric_limits<float>::infinity()) {
                        growthLimits[track] += plannedIncrease;
                    }

                    // correct growth limits
                    growthLimits[track] = std::max(growthLimits[track], baseSizes[track]);
                }

            }

        }

        // next is the previous step repeated for flexible tracks

        for (auto pass : passes) {
            std::map<int, float> baseSizePlannedIncreases {};
            std::map<int, float> growthLimitPlannedIncreases {};

            for (auto i = 0; i < this->items.size(); ++i) {
                auto& item = items[i];

                // why is this type optional; fix colstart/rowstart optionaltiy (llm's job)
                uint32_t start = isCol ? *item.placement.colStart : *item.placement.rowStart; // hate this selection method
                uint32_t end = isCol ? *item.placement.colEnd : *item.placement.rowEnd;
                uint32_t span = end - start;

                // init affected tracks
                std::set<int> affectedTracks {};

                // gather extra space contributions

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
                float spanGrowthLimitSpace = std::ranges::fold_left(std::span(growthLimits).subspan(start, end), 0, std::plus{});
                float baseSizeExtraSpace = 0.0f;
                float growthExtraSpace = 0.0f;

                for (auto spannedTrack : std::views::iota(start, end )) {
                    // only count tracks s.t. they're counted in pass?
                    // so maybe make this spannedTracks
                    // vs. affectedTracks
                    // then add to the affectedTracks set in the visitor
                    // and hten divide by that after

                    auto minSizingFunction = minSizingFunctions[spannedTrack];
                    auto maxSizingFunction = maxSizingFunctions[spannedTrack];

                    auto frTrack = std::holds_alternative<Size>(maxSizingFunction) ? std::get<Size>(maxSizingFunction).isFr() : false;
                    if (!frTrack) {
                        continue;
                    }

                    std::visit(Overloaded{
                        [&](Size& size){
                            if (pass == intrinsicMinimums && size.isFr()) {
                                // 1. for intrinsic minimums
                                float intrinsicContribution = 0.0f;
                                if (containerHasMinContentConstraint) {
                                    float limitedMinContent = minContent;
                                    if (std::holds_alternative<float>(maxSizingFunctions[start])) {
                                        float fixedTrackMaximum = std::get<float>(maxSizingFunctions[start]);
                                        limitedMinContent = std::min(limitedMinContent, fixedTrackMaximum);
                                    }
                                    limitedMinContent = std::max(limitedMinContent, item.widthContributions.minimum);
                                    intrinsicContribution = minContribution;
                                }else {
                                    intrinsicContribution = minContribution;
                                }

                                baseSizeExtraSpace = std::max(baseSizeExtraSpace, intrinsicContribution - spanBaseSize);
                            }else if(pass == contentBasedMinimums && size.isFr()) {
                                // 2. for content based minimums
                                baseSizeExtraSpace = std::max(baseSizeExtraSpace, minContent - spanBaseSize);
                            }else if (pass == maxContentMinimums && size.isFr()) {
                                // 3. for max-content minimums
                                baseSizeExtraSpace = std::max(baseSizeExtraSpace, maxContent - spanBaseSize);
                            }
                        },
                        [&](auto&) {
                            // dont alter intrinsic growth of base sizes/growth limits
                        }
                    }, maxSizingFunction);

                    std::visit(Overloaded{
                        [&](Size& size){
                            // intrinsic; according to the grid algo, is:
                            // isAuto() || isContentDependent()
                            if (pass == intrinsicMaximums && size.isFr()) {
                                growthExtraSpace = std::max(growthExtraSpace, minContent - spanGrowthLimitSpace);
                            }else if (pass == maxContentMaximums && size.isFr()) {
                                growthExtraSpace = std::max(growthExtraSpace, maxContent - spanGrowthLimitSpace);
                            }

                            if (size.isFr()) {
                                affectedTracks.insert(spannedTrack);
                            }
                        },
                        [&](auto&) {
                            // dont alter intrinsic growth of base sizes/growth limits
                        }
                    }, maxSizingFunction);

                    // baseSizePlannedIncreases[affectedTrack] = std::max(baseSizePlannedIncreases[affectedTrack], baseSizeExtraSpace / affectedTracks.size());
                    // growthLimitPlannedIncreases[affectedTrack] = std::max(growthLimitPlannedIncreases[affectedTrack], growthExtraSpace / affectedTracks.size());
                }

                float flexFactorSum = 0.0f;
                for (auto affectedTrack : affectedTracks) {
                    flexFactorSum += std::get<Size>(maxSizingFunctions[affectedTrack]).value;
                }

                for (auto affectedTrack : affectedTracks) {
                    auto flexFactor = std::get<Size>(maxSizingFunctions[affectedTrack]).value;
                    float proportion = flexFactorSum >= 1.0f ? flexFactor / flexFactorSum : flexFactor + (1.0f - flexFactorSum) / affectedTracks.size();
                    baseSizePlannedIncreases[affectedTrack] = std::max(baseSizePlannedIncreases[affectedTrack], baseSizeExtraSpace * proportion);
                    growthLimitPlannedIncreases[affectedTrack] = std::max(growthLimitPlannedIncreases[affectedTrack], growthExtraSpace * proportion);
                }


                // mark infinitely growable? I dont really understand their explanation as to *why*

            }

            // distribute extra space to tracks? does this need to move, idrk or think so

            for (auto& [track, plannedIncrease] : baseSizePlannedIncreases) {
                baseSizes[track] += plannedIncrease;
            }

            for (auto& [track, plannedIncrease] : growthLimitPlannedIncreases) {
                // how does something go from infinite to... finite?
                if (growthLimits[track] != std::numeric_limits<float>::infinity()) {
                    growthLimits[track] += plannedIncrease;
                }

                // correct growth limits
                growthLimits[track] = std::max(growthLimits[track], baseSizes[track]);
            }

        }
        
        // at the end, just set all growth limits with infinite size to base size?
        for (auto i = 0; i < growthLimits.size(); ++i) {
            if (growthLimits[i] == std::numeric_limits<float>::infinity()) {
                growthLimits[i] = baseSizes[i];
            }
        }

        // phase 4/5 depend on free space calc
        auto freeSpace = std::visit(Overloaded{
            [&](float resolved) -> SizeState {
                return std::max(0.0f, resolved - std::ranges::fold_left(baseSizes, 0.0f, std::plus{})); // also sub gaps
            },
            [&](auto& other) -> SizeState {
                return other;
            }
        }, available);

        // phase 4: maximize tracks
        /*
            Equal to the available grid space minus the sum of the base sizes of all the grid tracks (including gutters), floored at zero. 
            If available grid space is indefinite, the free space is indefinite as well.
        */

        /*
            other cases to shore up:
            For the purpose of this step: if sizing the grid container under a max-content constraint, the free space is infinite; if sizing under a min-content constraint, the free space is zero.

            If this would cause the grid to be larger than the grid container’s inner size as limited by its max-width/height, 
            then redo this step, treating the available grid space as equal to the grid container’s inner size when it’s sized 
            to its max-width/height. (for this I need the size state arg)
        */

        if (std::holds_alternative<float>(freeSpace)) {
            float resolvedFreeSpace = std::get<float>(freeSpace);
            for (auto [baseSize, growthLimit] : std::ranges::views::zip(baseSizes, growthLimits)) {
                baseSize = std::min(baseSize + resolvedFreeSpace / baseSizes.size(), growthLimit);
            }
        }

        // update free space (base sizes changed)
        freeSpace = std::visit(Overloaded{
            [&](float resolved) -> SizeState {
                return std::max(0.0f, resolved - std::ranges::fold_left(baseSizes, 0.0f, std::plus{})); // also sub gaps
            },
            [&](auto& other) -> SizeState {
                return other;
            }
        }, available);

        // phase 5: expand flexible tracks
        float flexFraction = 0.0f;

        if (std::holds_alternative<float>(freeSpace)) {
            auto resolvedFreeSpace = std::get<float>(freeSpace);
            if (resolvedFreeSpace > 0.0f) {
                auto spaceToFill = std::get<float>(available);
                auto leftoverSpace = spaceToFill; // needs to include gap?
                std::set<uint32_t> flexibleTracks {};

                for (uint32_t track = 0; track < numTracks; ++track) {
                    auto maxSizingFunction = maxSizingFunctions[track];
                    std::visit(Overloaded{
                        [&](Size& size) {
                            if (size.isFr()) {
                                flexibleTracks.insert(track);
                            }else {
                                leftoverSpace -= baseSizes[track];
                            }
                        },
                        [&](auto&) {
                            leftoverSpace -= baseSizes[track];
                        }
                    }, maxSizingFunction);
                }

                while (!flexibleTracks.empty()) {
                    float flexFactorSum = 0.0f;
                    for (auto track : flexibleTracks) {
                        flexFactorSum += std::get<Size>(maxSizingFunctions[track]).value;
                    }

                    auto hypotheticalFrSize = leftoverSpace / std::max(1.0f, flexFactorSum);
                    std::set<uint32_t> inflexibleTracks {};
                    for (auto track : flexibleTracks) {
                        auto flexFactor = std::get<Size>(maxSizingFunctions[track]).value;
                        if (hypotheticalFrSize * flexFactor < baseSizes[track]) {
                            inflexibleTracks.insert(track);
                        }
                    }

                    if (inflexibleTracks.empty()) {
                        flexFraction = hypotheticalFrSize;
                        break;
                    }

                    for (auto track : inflexibleTracks) {
                        flexibleTracks.erase(track);
                        leftoverSpace -= baseSizes[track];
                    }
                }
            }
        }else {

            if (!containerHasMinContentConstraint) {
                for (uint32_t track = 0; track < numTracks; ++track) {
                    auto maxSizingFunction = maxSizingFunctions[track];
                    std::visit(Overloaded{
                        [&](Size& size) {
                            if (size.isFr()) {
                                flexFraction = std::max(flexFraction, size.value > 1.0f ? baseSizes[track] / size.value : baseSizes[track]);
                            }
                        },
                        [&](auto&) {}
                    }, maxSizingFunction);
                }

                for (auto& item : items) {
                    uint32_t start = isCol ? *item.placement.colStart : *item.placement.rowStart;
                    uint32_t end = isCol ? *item.placement.colEnd : *item.placement.rowEnd;
                    auto spaceToFill = isCol ? item.widthContributions.maxContent : item.heightContributions.maxContent;
                    auto leftoverSpace = spaceToFill; // needs to include gap
                    std::set<uint32_t> flexibleTracks {};

                    for (auto track = start; track < end; ++track) {
                        auto maxSizingFunction = maxSizingFunctions[track];
                        std::visit(Overloaded{
                            [&](Size& size) {
                                if (size.isFr()) {
                                    flexibleTracks.insert(track);
                                }else {
                                    leftoverSpace -= baseSizes[track];
                                }
                            },
                            [&](auto&) {
                                leftoverSpace -= baseSizes[track];
                            }
                        }, maxSizingFunction);
                    }

                    while (!flexibleTracks.empty()) {
                        float flexFactorSum = 0.0f;
                        for (auto track : flexibleTracks) {
                            flexFactorSum += std::get<Size>(maxSizingFunctions[track]).value;
                        }

                        auto hypotheticalFrSize = leftoverSpace / std::max(1.0f, flexFactorSum);
                        std::set<uint32_t> inflexibleTracks {};
                        for (auto track : flexibleTracks) {
                            auto flexFactor = std::get<Size>(maxSizingFunctions[track]).value;
                            if (hypotheticalFrSize * flexFactor < baseSizes[track]) {
                                inflexibleTracks.insert(track);
                            }
                        }

                        if (inflexibleTracks.empty()) {
                            flexFraction = std::max(flexFraction, hypotheticalFrSize);
                            break;
                        }

                        for (auto track : inflexibleTracks) {
                            flexibleTracks.erase(track);
                            leftoverSpace -= baseSizes[track];
                        }
                    }
                }
            }
        }

        for (uint32_t track = 0; track < numTracks; ++track) {
            auto maxSizingFunction = maxSizingFunctions[track];
            std::visit(Overloaded{
                [&](Size& size) {
                    if (size.isFr()) {
                        baseSizes[track] = std::max(baseSizes[track], flexFraction * size.value);
                    }
                },
                [&](auto&) {}
            }, maxSizingFunction);
        }
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
