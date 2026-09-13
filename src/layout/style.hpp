#pragma once

#include "layout/sizing.hpp"
#include "metal_imports.hpp"
#include <optional>
#include <string>
#include <vector>

namespace style {
    enum class Position {
        Absolute,
        Fixed,
        Static,
        Relative,
    };

    enum class Display {
        Block,
        Inline,
        Flex,
        Grid
    };

    enum class FlexDirection {
        Row,
        Col,
        RowReverse,
        ColReverse
    };

    enum class JustifyContent {
        FlexStart,
        FlexEnd,
        Center,
        SpaceBetween,
        SpaceAround,
        SpaceEvenly,
        Normal,
        Stretch,
        Start,
        End
    };

    enum class AlignItems {
        Stretch,
        FlexStart,
        FlexEnd,
        Center,
    };

    enum class FlexWrap {
        NoWrap,
        Wrap,
        WrapReverse
    };

    enum class AlignContent {
        Stretch,
        FlexStart,
        FlexEnd,
        Center,
        SpaceBetween,
        SpaceAround,
        SpaceEvenly,
        Normal,
        Start,
        End
    };

    enum class AlignSelf {
        Auto,
        Stretch,
        FlexStart,
        FlexEnd,
        Center
    };

    enum class JustifyItems {
        Stretch,
        Start,
        End,
        Center
    };

    enum class JustifySelf {
        Auto,
        Stretch,
        Start,
        End,
        Center
    };

    enum class Overflow {
        Visible,
        Hidden,
        Scroll
    };

    enum class PointerEvents {
        Auto,
        None
    };

    enum class WhiteSpace {
        Normal,
        NoWrap,
        Pre,
        PreWrap
    };

    enum class WordBreak {
        Normal,
        BreakAll
    };

    enum class TextAlign {
        Start,
        Left,
        Center,
        Right
    };

    struct TextOverflow {
        enum class Mode {
            Clip,
            Ellipsis,
            Custom
        };

        Mode mode{Mode::Clip};
        std::string ending{};

        static TextOverflow clip() {
            return {};
        }

        static TextOverflow ellipsis() {
            return {.mode = Mode::Ellipsis, .ending = "\xE2\x80\xA6"};
        }

        static TextOverflow custom(std::string ending) {
            return {.mode = Mode::Custom, .ending = ending};
        }

        bool drawsEnding() const {
            return mode != Mode::Clip && !ending.empty();
        }
    };

    struct ClipUniform {
        simd_float2 rectCenter{};
        simd_float2 halfExtent{};
        simd_float2 cornerRadius{};
    };

    struct GridPlacement {
        int colStart{0};  // 1-based line number, 0 = auto
        int colEnd{0};    // 0 = colStart+1 (span 1)
        int rowStart{0};
        int rowEnd{0};
    };

    struct SharedDescriptor {
        Position position{Position::Static};
        Display display{Display::Block};

        Size width{Size::autoSize()};
        Size height{Size::autoSize()};
        Size minWidth{Size::autoSize()};
        Size minHeight{Size::autoSize()};
        std::optional<Size> maxWidth, maxHeight;
        std::optional<float> aspectRatio;
        std::optional<Size> top, left, bottom, right;

        Size margin{};
        std::optional<Size> marginLeft, marginRight, marginTop, marginBottom;

        Size padding{};
        std::optional<Size> paddingLeft, paddingRight, paddingTop, paddingBottom;

        FlexDirection flexDirection{FlexDirection::Row};
        JustifyContent justifyContent{JustifyContent::FlexStart};
        AlignItems alignItems{AlignItems::Stretch};
        FlexWrap flexWrap{FlexWrap::NoWrap};
        AlignContent alignContent{AlignContent::Stretch};
        AlignSelf alignSelf{AlignSelf::Auto};
        JustifyItems justifyItems{JustifyItems::Stretch};
        JustifySelf justifySelf{JustifySelf::Auto};
        Size flexGrow{Size::px(0.0)};
        Size flexShrink{Size::px(1.0)};
        Size flexGap{Size::px(0)};

        // Grid container properties
        std::vector<Size> gridTemplateColumns{};
        std::vector<Size> gridTemplateRows{};
        Size gridColumnGap{Size::px(0)};
        Size gridRowGap{Size::px(0)};

        // Grid item properties
        GridPlacement gridPlacement{};

        Size cornerRadius{};
        Size borderWidth{};
        simd_float4 borderColor{0,0,0,1};

        Overflow overflow {Overflow::Visible};
        PointerEvents pointerEvents {PointerEvents::Auto};
        TextOverflow textOverflow{};
        std::optional<TextAlign> textAlign{};
    };
}
