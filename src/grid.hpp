#pragma once

#include "element.hpp"
#include "new_sizing.hpp"
#include <optional>
#include <unordered_map>
#include <vector>

namespace tree {
    struct RenderTree;
}

namespace layout {
    using style::AlignItems;
    using style::JustifyItems;
    using style::JustifySelf;
    using style::Size;
    using tree::RenderTree;
    using tree::TreeNode;

    struct ItemPlacement {
        std::optional<int> colStart;
        std::optional<int> colEnd;
        std::optional<int> rowStart;
        std::optional<int> rowEnd;

        bool colNeedsResolution() {
            return !(colStart.has_value() && colEnd.has_value());
        }

        bool rowNeedsResolution() {
            return !(rowStart.has_value() && rowEnd.has_value());
        }
    };

    struct GridItemContributions {
        float minimum;
        float minContent;
        float maxContent;
    };

    struct GridItem {
        size_t childIndex;
        ItemPlacement placement;
        GridItemContributions widthContributions;
        GridItemContributions heightContributions;
    };

    enum class GridDirection {
        Col,
        Row
    };

    struct Grid {
        std::vector<std::vector<uint8_t>> occupied; // [row][col]
        size_t numRows;
        size_t numCols;
        GridDirection majorAxis;

        int cursorMajor {};
        int cursorMinor {};

        Grid(size_t rows, size_t cols, GridDirection major = GridDirection::Row);

        void mark(int row, int col);
        bool regionFree(int row, int col, int spanRows, int spanCols) const;
        void growMajor(int needed);
        void advanceCursor(int spanMinor);

        std::pair<int, int> findSpace(int spanRows, int spanCols);

        int majorSize() const;
        int minorSize() const;
    };

    struct Track {
        float offset;
        float size;
    };

    struct GridLayout {
        std::vector<GridItem> items;
        Grid grid {0, 0};
        std::vector<Track> rowTracks;
        std::vector<Track> colTracks;
        IntrinsicSizes columnIntrinsicSizes;
        IntrinsicSizes rowIntrinsicSizes;

        void addChild(size_t childIndex, TreeNode* node, GridItemContributions widthContributions);

        // helpers
        void resolveStructure(size_t templateRows, size_t templateCols);
        std::vector<Track> resolveTracks(
            std::vector<Size>& templateTracks,
            const SizeState& available,
            float gap,
            bool isCol,
            IntrinsicSizes* intrinsicSizes = nullptr
        );

        void resolveColumns(size_t numRows, size_t numCols, const std::vector<Size>& templateCols, const SizeState& availableWidth, float colGap);
        void resolveRows(const std::vector<Size>& templateRows, const SizeState& availableHeight, float rowGap);
    };

    struct GridResolver {
        RenderTree&       tree;
        TreeNode*         node;
        Constraints       parentConstraints;
        Constraints       childConstraints;
        GridLayout        gridLayout;
        AlignItems        alignItems;
        JustifyItems      justifyItems;
        const FrameInfo&  frameInfo;
        const SizePair&   availableSize;
        bool              mutate;
        std::unordered_map<size_t, SizeResult>& sizeCache;

        float minX;
        float minY;
        float maxX;
        float maxY;

        struct Bounds {
            float maxX;
            float maxY;
        };

        GridResolver(RenderTree& tree, TreeNode* node,
                     const Constraints& parentConstraints,
                     const Constraints& childConstraints,
                     const FrameInfo& frameInfo,
                     const SizePair& availableSize, bool mutate,
                     std::unordered_map<size_t, SizeResult>& sizeCache,
                     float minX, float minY, float maxX, float maxY);

        Constraints prepareChildConstraints();

        void phaseB();
        Bounds phaseC();
    };
}
