#pragma once

#include "layout/sizing.hpp"
#include "metal_imports.hpp"
#include "layout/style.hpp"
#include <algorithm>
#include <optional>
#include <simd/matrix.h>
#include <string>
#include <vector>

namespace style {
    enum class Position {
        Absolute,
        Fixed,
        Static,
        Relative,
        Sticky,
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

    struct CornerRadii {
        simd_float2 topLeft{};
        simd_float2 topRight{};
        simd_float2 bottomRight{};
        simd_float2 bottomLeft{};
    };

    struct ClipUniform {
        simd_float2 rectCenter{};
        simd_float2 halfExtent{};
        CornerRadii cornerRadius{};
        simd_float3x3 inverseTransform {matrix_identity_float3x3};
    };

    enum class BorderStyle : uint32_t {
        Solid = 0,
        Dashed = 1,
        Dotted = 2,
        Double = 3,
    };

    struct Border {
        Size width{};
        simd_float4 color{0, 0, 0, 1};
        BorderStyle style{BorderStyle::Solid};
    };

    struct BorderUniform {
        float width{0};
        simd_float4 color{0, 0, 0, 1};
        BorderStyle style{BorderStyle::Solid};
    };

    struct BoxShadow {
        Size offsetX{};
        Size offsetY{};
        Size blur{};
        Size spread{};
        simd_float4 color{0, 0, 0, 0};
        bool inset{false};
    };

    struct ShadowUniform {
        simd_float2 offset{};
        float spread{0};
        float sigma{0};
        simd_float4 color{0, 0, 0, 0};
        uint32_t inset{0};
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
        std::optional<Size> cornerRadiusTopLeft, cornerRadiusTopRight;
        std::optional<Size> cornerRadiusBottomRight, cornerRadiusBottomLeft;
        Border border{};
        BoxShadow boxShadow{};

        Overflow overflow {Overflow::Visible};
        PointerEvents pointerEvents {PointerEvents::Auto};
        TextOverflow textOverflow{};
        std::optional<TextAlign> textAlign{};

        // the reason its optional here is bc 
        // the spec distinguishes between the identity (i.e. set via scale(1))
        // and a literal no transform set for establishing containing blocks (no transform -> dont establish)
        std::optional<simd_float3x3> transform {};
        std::optional<simd_float2> translate {};
        std::optional<float> rotate {};
        std::optional<simd_float2> scale {};
    };

    inline auto resolveCornerRadii(const SharedDescriptor& desc, float width, float height) -> CornerRadii {
        auto topLeft = desc.cornerRadiusTopLeft.value_or(desc.cornerRadius);
        auto topRight = desc.cornerRadiusTopRight.value_or(desc.cornerRadius);
        auto bottomLeft = desc.cornerRadiusBottomLeft.value_or(desc.cornerRadius);
        auto bottomRight = desc.cornerRadiusBottomRight.value_or(desc.cornerRadius);

        // first; resolve the corner radius normally
        CornerRadii cornerRadii {
            .topLeft = {topLeft.resolveOr(Size::px(width), 0.0), topLeft.resolveOr(Size::px(height), 0.0)},
            .topRight = {topRight.resolveOr(Size::px(width), 0.0), topRight.resolveOr(Size::px(height), 0.0)},
            .bottomRight = {bottomRight.resolveOr(Size::px(width), 0.0), bottomRight.resolveOr(Size::px(height), 0.0)},
            .bottomLeft = {bottomLeft.resolveOr(Size::px(width), 0.0), bottomLeft.resolveOr(Size::px(height), 0.0)}
        };

        /*
            then; this is for the pill case
            lets say the corner radius execeeds one dimension
            (common when the corner radius >= height)

            then, we'll scale both down uniformly according to this function
            which is defined roughly equivalently in CSS
        */
        float topSum = cornerRadii.topLeft.x + cornerRadii.topRight.x;
        float bottomSum = cornerRadii.bottomLeft.x + cornerRadii.bottomRight.x;
        float leftSum = cornerRadii.topLeft.y + cornerRadii.bottomLeft.y;
        float rightSum = cornerRadii.topRight.y + cornerRadii.bottomRight.y;

        float radiusScale = 1.0f;

        if (topSum > width) {
            radiusScale = std::min(radiusScale, width / topSum);
        }

        if (bottomSum > width) {
            radiusScale = std::min(radiusScale, width / bottomSum);
        }

        if (leftSum > height) {
            radiusScale = std::min(radiusScale, height / leftSum);
        }

        if (rightSum > height) {
            radiusScale = std::min(radiusScale, height / rightSum);
        }

        cornerRadii.topLeft *= radiusScale;
        cornerRadii.topRight *= radiusScale;
        cornerRadii.bottomRight *= radiusScale;
        cornerRadii.bottomLeft *= radiusScale;

        return cornerRadii;
    }
}
