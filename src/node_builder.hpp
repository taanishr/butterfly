#pragma once

#include "tree_node.hpp"
#include "primitives/div.hpp"
#include "events.hpp"
#include "fonts.hpp"
#include "renderer.hpp"
#include "primitives/image.hpp"
#include "primitives/svg.hpp"
#include "primitives/text.hpp"
#include "layout/layout.hpp"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <print>
#include <source_location>
#include "context_manager.hpp"
#include "simd_helpers.hpp"

namespace elements {
    using runtime::ContextManager;
    using runtime::Event;
    using runtime::EventType;
    using runtime::UIContext;
    using style::AlignContent;
    using style::AlignItems;
    using style::AlignSelf;
    using style::BorderStyle;
    using style::Display;
    using style::FlexDirection;
    using style::FlexWrap;
    using style::GridPlacement;
    using style::JustifyContent;
    using style::JustifyItems;
    using style::JustifySelf;
    using style::Overflow;
    using style::PointerEvents;
    using style::Position;
    using style::Size;
    using style::TextOverflow;
    using style::TextAlign;
    using style::WhiteSpace;
    using style::WordBreak;
    using tree::RenderTree;
    using tree::DirtyBits;
    using tree::TreeNode;

    // Element-specific concepts (only for properties that vary by element type)
    template<typename T>
    concept HasText = requires { std::declval<T&>().text; };

    template<typename E>
    concept LeafElement = requires { requires E::isLeaf; };

    template <typename Derived, ElementType E, typename P>
        requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
    struct NodeMutation {
    protected:
        RenderTree& renderTree;
        TreeNode* node;

    public:
        NodeMutation(RenderTree& tree, TreeNode* node):
            renderTree{tree},
            node{node}
        {}

        void markDirty(std::source_location source = std::source_location::current()) {
            renderTree.markDirty(source);
        }

        void markDirty(
            DirtyBits bits,
            std::source_location source = std::source_location::current()
        ) {
            renderTree.markDirty(node, bits, source);
        }

        static constexpr DirtyBits layoutDirtyBits() {
            return DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout |
                DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize;
        }

        static constexpr DirtyBits textDirtyBits() {
            return DirtyBits::Atomize | DirtyBits::Layout |
                DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize | DirtyBits::PaintOrder;
        }
        Derived& self() {
            return static_cast<Derived&>(*this);
        }

        const Derived& self() const {
            return static_cast<const Derived&>(*this);
        }

        Position position() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.position;
        }

        Derived& position(Position position) requires (!HasText<typename E::DescriptorType>) {
            node->shared.position = position;
            markDirty(layoutDirtyBits() | DirtyBits::PaintOrder);
            return self();
        }

        Display display() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.display;
        }

        Derived& display(Display display) requires (!HasText<typename E::DescriptorType>) {
            node->shared.display = display;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size width() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.width;
        }

        Derived& width(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.width = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& animateWidth(Size target, double duration, animation::Easing easing = animation::Easing::Linear, std::optional<Size> from = std::nullopt) requires (!HasText<typename E::DescriptorType>) {
            auto& manager = renderTree.animationManager;
            auto& slot = node->animations[animation::Animatable::Width];
            Size initial = from.value_or(manager.width(slot, node->getWidth()));

            if (initial.unit == target.unit) {
                slot = animation::AnimationState{manager.now, manager.now + duration, easing, initial};
            } else {
                slot.reset();
            }

            return width(target);
        }

        Size minWidth() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.minWidth;
        }

        Derived& minWidth(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.minWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> maxWidth() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.maxWidth;
        }

        Derived& maxWidth(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.maxWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size height() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.height;
        }

        Derived& height(Size height) requires (!HasText<typename E::DescriptorType>) {
            node->shared.height = height;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& animateHeight(Size target, double duration, animation::Easing easing = animation::Easing::Linear, std::optional<Size> from = std::nullopt) requires (!HasText<typename E::DescriptorType>) {
            auto& manager = renderTree.animationManager;
            auto& slot = node->animations[animation::Animatable::Height];
            Size initial = from.value_or(manager.height(slot, node->getHeight()));

            if (initial.unit == target.unit) {
                slot = animation::AnimationState{manager.now, manager.now + duration, easing, initial};
            } else {
                slot.reset();
            }

            return height(target);
        }

        Derived& aspectRatio(float width, float height = 1.0f) requires (!HasText<typename E::DescriptorType>) {
            node->shared.aspectRatio =
                width > 0.0f && height > 0.0f
                    ? std::optional<float>{width / height}
                    : std::nullopt;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size minHeight() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.minHeight;
        }

        Derived& minHeight(Size height) requires (!HasText<typename E::DescriptorType>) {
            node->shared.minHeight = height;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> maxHeight() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.maxHeight;
        }

        Derived& maxHeight(Size height) requires (!HasText<typename E::DescriptorType>) {
            node->shared.maxHeight = height;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> top() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.top;
        }

        Derived& top(Size top) requires (!HasText<typename E::DescriptorType>) {
            node->shared.top = top;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> right() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.right;
        }

        Derived& right(Size right) requires (!HasText<typename E::DescriptorType>) {
            node->shared.right = right;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> bottom() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.bottom;
        }

        Derived& bottom(Size bottom) requires (!HasText<typename E::DescriptorType>) {
            node->shared.bottom = bottom;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> left() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.left;
        }

        Derived& left(Size left) requires (!HasText<typename E::DescriptorType>) {
            node->shared.left = left;
            markDirty(layoutDirtyBits());
            return self();
        }

        FlexDirection flexDirection() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.flexDirection;
        }

        Derived& flexDirection(FlexDirection direction) requires (!HasText<typename E::DescriptorType>) {
            node->shared.flexDirection = direction;
            markDirty(layoutDirtyBits());
            return self();
        }

        JustifyContent justifyContent() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.justifyContent;
        }

        Derived& justifyContent(JustifyContent justifyContent) requires (!HasText<typename E::DescriptorType>) {
            node->shared.justifyContent = justifyContent;
            markDirty(layoutDirtyBits());
            return self();
        }

        AlignItems alignItems() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.alignItems;
        }

        Derived& alignItems(AlignItems alignItems) requires (!HasText<typename E::DescriptorType>) {
            node->shared.alignItems = alignItems;
            markDirty(layoutDirtyBits());
            return self();
        }

        FlexWrap flexWrap() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.flexWrap;
        }

        Derived& flexWrap(FlexWrap wrap) requires (!HasText<typename E::DescriptorType>) {
            node->shared.flexWrap = wrap;
            markDirty(layoutDirtyBits());
            return self();
        }

        AlignContent alignContent() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.alignContent;
        }

        Derived& alignContent(AlignContent ac) requires (!HasText<typename E::DescriptorType>) {
            node->shared.alignContent = ac;
            markDirty(layoutDirtyBits());
            return self();
        }

        AlignSelf alignSelf() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.alignSelf;
        }

        Derived& alignSelf(AlignSelf as) requires (!HasText<typename E::DescriptorType>) {
            node->shared.alignSelf = as;
            markDirty(layoutDirtyBits());
            return self();
        }

        JustifyItems justifyItems() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.justifyItems;
        }

        Derived& justifyItems(JustifyItems justifyItems) requires (!HasText<typename E::DescriptorType>) {
            node->shared.justifyItems = justifyItems;
            markDirty(layoutDirtyBits());
            return self();
        }

        JustifySelf justifySelf() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.justifySelf;
        }

        Derived& justifySelf(JustifySelf justifySelf) requires (!HasText<typename E::DescriptorType>) {
            node->shared.justifySelf = justifySelf;
            markDirty(layoutDirtyBits());
            return self();
        }

        Overflow overflow() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.overflow;
        }

        Derived& overflow(Overflow overflow) requires (!HasText<typename E::DescriptorType>) {
            node->shared.overflow = overflow;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<simd_float2> translate() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.translate;
        }

        Derived& translate(simd_float2 translate) requires (!HasText<typename E::DescriptorType>) {
            auto& shared = node->shared;
            shared.translate = translate;
            shared.transform = composeTransform(
                translate,
                shared.rotate.value_or(0.0f),
                shared.scale.value_or(simd_float2{1.0f, 1.0f})
            );
            markDirty(DirtyBits::PostLayout | DirtyBits::Finalize | DirtyBits::PaintOrder);
            return self();
        }

        Derived& animateTranslate(simd_float2 target, double duration, animation::Easing easing = animation::Easing::Linear, std::optional<simd_float2> from = std::nullopt) requires (!HasText<typename E::DescriptorType>) {
            auto& manager = renderTree.animationManager;
            auto& slot = node->animations[animation::Animatable::Translate];
            simd_float2 initial = from.value_or(manager.transform(slot, node->getTransform()).columns[2].xy);

            slot = animation::AnimationState{manager.now, manager.now + duration, easing, initial};

            return translate(target);
        }

        std::optional<float> rotate() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.rotate;
        }

        Derived& rotate(float rotate) requires (!HasText<typename E::DescriptorType>) {
            auto& shared = node->shared;
            shared.rotate = rotate;
            shared.transform = composeTransform(
                shared.translate.value_or(simd_float2{0.0f, 0.0f}),
                rotate,
                shared.scale.value_or(simd_float2{1.0f, 1.0f})
            );
            markDirty(DirtyBits::PostLayout | DirtyBits::Finalize | DirtyBits::PaintOrder);
            return self();
        }

        std::optional<simd_float2> scale() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.scale;
        }

        Derived& scale(simd_float2 scale) requires (!HasText<typename E::DescriptorType>) {
            auto& shared = node->shared;
            shared.scale = scale;
            shared.transform = composeTransform(
                shared.translate.value_or(simd_float2{0.0f, 0.0f}),
                shared.rotate.value_or(0.0f),
                scale
            );
            markDirty(DirtyBits::PostLayout | DirtyBits::Finalize | DirtyBits::PaintOrder);
            return self();
        }

        std::pair<Size, Size> transformOrigin() const requires (!HasText<typename E::DescriptorType>) {
            return {node->shared.transformOriginX, node->shared.transformOriginY};
        }

        Derived& transformOrigin(Size x, Size y) requires (!HasText<typename E::DescriptorType>) {
            node->shared.transformOriginX = x;
            node->shared.transformOriginY = y;
            markDirty(DirtyBits::PostLayout | DirtyBits::Finalize);
            return self();
        }

        float opacity() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.opacity;
        }

        Derived& opacity(float opacity) requires (!HasText<typename E::DescriptorType>) {
            node->shared.opacity = std::clamp(opacity, 0.0f, 1.0f);
            markDirty(DirtyBits::PostLayout | DirtyBits::Finalize | DirtyBits::PaintOrder);
            return self();
        }

        Derived& animateOpacity(float target, double duration, animation::Easing easing = animation::Easing::Linear, std::optional<float> from = std::nullopt) requires (!HasText<typename E::DescriptorType>) {
            auto& manager = renderTree.animationManager;
            auto& slot = node->animations[animation::Animatable::Opacity];
            float initial = from.value_or(manager.opacity(slot, node->getOpacity()));

            slot = animation::AnimationState{manager.now, manager.now + duration, easing, initial};

            return opacity(target);
        }

        std::optional<simd_float3x3> transform() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.transform;
        }

        Derived& transform(std::optional<simd_float3x3> transform) requires (!HasText<typename E::DescriptorType>) {
            node->shared.transform = transform;
            markDirty(layoutDirtyBits() | DirtyBits::PaintOrder);
            return self();
        }

        PointerEvents pointerEvents() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.pointerEvents;
        }

        Derived& pointerEvents(PointerEvents pointerEvents) requires (!HasText<typename E::DescriptorType>) {
            node->shared.pointerEvents = pointerEvents;
            return self();
        }

        const TextOverflow& textOverflow() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.textOverflow;
        }

        Derived& textOverflow(TextOverflow textOverflow) requires (!HasText<typename E::DescriptorType>) {
            node->shared.textOverflow = std::move(textOverflow);
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<TextAlign> textAlign() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.textAlign;
        }

        Derived& textAlign(TextAlign textAlign) requires (!HasText<typename E::DescriptorType>) {
            node->shared.textAlign = textAlign;
            markDirty(DirtyBits::Layout | DirtyBits::PostLayout |
                      DirtyBits::Place | DirtyBits::Finalize);
            return self();
        }

        Size flexGrow() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.flexGrow;
        }

        Derived& flexGrow(Size grow) requires (!HasText<typename E::DescriptorType>) {
            node->shared.flexGrow = grow;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size flexGap() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.flexGap;
        }

        Derived& flexGap(Size gap) requires (!HasText<typename E::DescriptorType>) {
            node->shared.flexGap = gap;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size flexShrink() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.flexShrink;
        }

        Derived& flexShrink(Size shrink) requires (!HasText<typename E::DescriptorType>) {
            node->shared.flexShrink = shrink;
            markDirty(layoutDirtyBits());
            return self();
        }

        const std::vector<Size>& gridTemplateColumns() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.gridTemplateColumns;
        }

        Derived& gridTemplateColumns(std::vector<Size> tracks) requires (!HasText<typename E::DescriptorType>) {
            node->shared.gridTemplateColumns = std::move(tracks);
            markDirty(layoutDirtyBits());
            return self();
        }

        const std::vector<Size>& gridTemplateRows() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.gridTemplateRows;
        }

        Derived& gridTemplateRows(std::vector<Size> tracks) requires (!HasText<typename E::DescriptorType>) {
            node->shared.gridTemplateRows = std::move(tracks);
            markDirty(layoutDirtyBits());
            return self();
        }

        Size gridColumnGap() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.gridColumnGap;
        }

        Derived& gridColumnGap(Size gap) requires (!HasText<typename E::DescriptorType>) {
            node->shared.gridColumnGap = gap;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size gridRowGap() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.gridRowGap;
        }

        Derived& gridRowGap(Size gap) requires (!HasText<typename E::DescriptorType>) {
            node->shared.gridRowGap = gap;
            markDirty(layoutDirtyBits());
            return self();
        }

        GridPlacement gridPlacement() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.gridPlacement;
        }

        Derived& gridColumn(int start, int end = 0) requires (!HasText<typename E::DescriptorType>) {
            node->shared.gridPlacement.colStart = start;
            node->shared.gridPlacement.colEnd = end;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& gridRow(int start, int end = 0) requires (!HasText<typename E::DescriptorType>) {
            node->shared.gridPlacement.rowStart = start;
            node->shared.gridPlacement.rowEnd = end;
            markDirty(layoutDirtyBits());
            return self();
        }

        Size cornerRadius() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.cornerRadius;
        }

        Derived& cornerRadius(Size radius) requires (!HasText<typename E::DescriptorType>) {
            node->shared.cornerRadius = radius;
            markDirty(DirtyBits::PostLayout);
            return self();
        }

        std::optional<Size> cornerRadiusTopLeft() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.cornerRadiusTopLeft;
        }

        Derived& cornerRadiusTopLeft(Size radius) requires (!HasText<typename E::DescriptorType>) {
            node->shared.cornerRadiusTopLeft = radius;
            markDirty(DirtyBits::PostLayout);
            return self();
        }

        std::optional<Size> cornerRadiusTopRight() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.cornerRadiusTopRight;
        }

        Derived& cornerRadiusTopRight(Size radius) requires (!HasText<typename E::DescriptorType>) {
            node->shared.cornerRadiusTopRight = radius;
            markDirty(DirtyBits::PostLayout);
            return self();
        }

        std::optional<Size> cornerRadiusBottomRight() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.cornerRadiusBottomRight;
        }

        Derived& cornerRadiusBottomRight(Size radius) requires (!HasText<typename E::DescriptorType>) {
            node->shared.cornerRadiusBottomRight = radius;
            markDirty(DirtyBits::PostLayout);
            return self();
        }

        std::optional<Size> cornerRadiusBottomLeft() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.cornerRadiusBottomLeft;
        }

        Derived& cornerRadiusBottomLeft(Size radius) requires (!HasText<typename E::DescriptorType>) {
            node->shared.cornerRadiusBottomLeft = radius;
            markDirty(DirtyBits::PostLayout);
            return self();
        }

        Size borderWidth() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.border.width;
        }

        Derived& borderWidth(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.border.width = width;
            node->shared.border.topWidth = width;
            node->shared.border.rightWidth = width;
            node->shared.border.bottomWidth = width;
            node->shared.border.leftWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> borderTopWidth() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.border.topWidth;
        }

        Derived& borderTopWidth(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.border.topWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> borderRightWidth() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.border.rightWidth;
        }

        Derived& borderRightWidth(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.border.rightWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> borderBottomWidth() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.border.bottomWidth;
        }

        Derived& borderBottomWidth(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.border.bottomWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> borderLeftWidth() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.border.leftWidth;
        }

        Derived& borderLeftWidth(Size width) requires (!HasText<typename E::DescriptorType>) {
            node->shared.border.leftWidth = width;
            markDirty(layoutDirtyBits());
            return self();
        }

        simd_float4 borderColor() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.border.color;
        }

        Derived& borderColor(simd_float4 color) requires (!HasText<typename E::DescriptorType>) {
            node->shared.border.color = color;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        BorderStyle borderStyle() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.border.style;
        }

        Derived& borderStyle(BorderStyle borderStyle) requires (!HasText<typename E::DescriptorType>) {
            node->shared.border.style = borderStyle;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Size shadowOffsetX() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.boxShadow.offsetX;
        }

        Derived& shadowOffsetX(Size offset) requires (!HasText<typename E::DescriptorType>) {
            node->shared.boxShadow.offsetX = offset;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Size shadowOffsetY() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.boxShadow.offsetY;
        }

        Derived& shadowOffsetY(Size offset) requires (!HasText<typename E::DescriptorType>) {
            node->shared.boxShadow.offsetY = offset;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Size shadowBlur() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.boxShadow.blur;
        }

        Derived& shadowBlur(Size blur) requires (!HasText<typename E::DescriptorType>) {
            node->shared.boxShadow.blur = blur;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Size shadowSpread() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.boxShadow.spread;
        }

        Derived& shadowSpread(Size spread) requires (!HasText<typename E::DescriptorType>) {
            node->shared.boxShadow.spread = spread;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        simd_float4 shadowColor() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.boxShadow.color;
        }

        Derived& shadowColor(simd_float4 color) requires (!HasText<typename E::DescriptorType>) {
            node->shared.boxShadow.color = color;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        bool shadowInset() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.boxShadow.inset;
        }

        Derived& shadowInset(bool inset) requires (!HasText<typename E::DescriptorType>) {
            node->shared.boxShadow.inset = inset;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Size padding() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.padding;
        }

        Derived& padding(Size padding) requires (!HasText<typename E::DescriptorType>) {
            node->shared.padding = padding;
            node->shared.paddingTop = padding;
            node->shared.paddingRight = padding;
            node->shared.paddingBottom = padding;
            node->shared.paddingLeft = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingTop() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.paddingTop;
        }

        Derived& paddingTop(Size padding) requires (!HasText<typename E::DescriptorType>) {
            node->shared.paddingTop = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingRight() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.paddingRight;
        }

        Derived& paddingRight(Size padding) requires (!HasText<typename E::DescriptorType>) {
            node->shared.paddingRight = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingBottom() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.paddingBottom;
        }

        Derived& paddingBottom(Size padding) requires (!HasText<typename E::DescriptorType>) {
            node->shared.paddingBottom = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<Size> paddingLeft() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.paddingLeft;
        }

        Derived& paddingLeft(Size padding) requires (!HasText<typename E::DescriptorType>) {
            node->shared.paddingLeft = padding;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& margin(float margin) requires (!HasText<typename E::DescriptorType>) {
            return this->margin(Size::px(margin));
        }

        Size margin() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.margin;
        }

        Derived& margin(Size margin) requires (!HasText<typename E::DescriptorType>) {
            node->shared.margin = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginTop(float margin) requires (!HasText<typename E::DescriptorType>) {
            return this->marginTop(Size::px(margin));
        }

        std::optional<Size> marginTop() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.marginTop;
        }

        Derived& marginTop(Size margin) requires (!HasText<typename E::DescriptorType>) {
            node->shared.marginTop = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginRight(float margin) requires (!HasText<typename E::DescriptorType>) {
            return this->marginRight(Size::px(margin));
        }

        std::optional<Size> marginRight() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.marginRight;
        }

        Derived& marginRight(Size margin) requires (!HasText<typename E::DescriptorType>) {
            node->shared.marginRight = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginBottom(float margin) requires (!HasText<typename E::DescriptorType>) {
            return this->marginBottom(Size::px(margin));
        }

        std::optional<Size> marginBottom() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.marginBottom;
        }

        Derived& marginBottom(Size margin) requires (!HasText<typename E::DescriptorType>) {
            node->shared.marginBottom = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        Derived& marginLeft(float margin) requires (!HasText<typename E::DescriptorType>) {
            return this->marginLeft(Size::px(margin));
        }

        std::optional<Size> marginLeft() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.marginLeft;
        }

        Derived& marginLeft(Size margin) requires (!HasText<typename E::DescriptorType>) {
            node->shared.marginLeft = margin;
            markDirty(layoutDirtyBits());
            return self();
        }

        std::optional<int64_t> zIndex() const requires (!HasText<typename E::DescriptorType>) {
            return node->zIndex;
        }

        Derived& zIndex(std::optional<int64_t> zIndex) requires (!HasText<typename E::DescriptorType>) {
            node->zIndex = zIndex;
            markDirty(DirtyBits::PaintOrder);
            return self();
        }

        simd_float4 backgroundColor() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.backgroundColor;
        }

        Derived& backgroundColor(simd_float4 backgroundColor) requires (!HasText<typename E::DescriptorType>) {
            node->shared.backgroundColor = backgroundColor;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Derived& animateBackgroundColor(simd_float4 target, double duration, animation::Easing easing = animation::Easing::Linear, std::optional<simd_float4> from = std::nullopt) requires (!HasText<typename E::DescriptorType>) {
            auto& manager = renderTree.animationManager;
            auto& slot = node->animations[animation::Animatable::BackgroundColor];
            simd_float4 initial = from.value_or(manager.color(slot, node->getBackgroundColor()));

            slot = animation::AnimationState{manager.now, manager.now + duration, easing, initial};

            return backgroundColor(target);
        }

        simd_float4 color() const requires (!HasText<typename E::DescriptorType>) {
            return node->getColor();
        }

        Derived& color(simd_float4 color) requires (!HasText<typename E::DescriptorType>) {
            node->shared.color = color;
            markDirty(DirtyBits::Finalize);
            return self();
        }

        Derived& animateColor(simd_float4 target, double duration, animation::Easing easing = animation::Easing::Linear, std::optional<simd_float4> from = std::nullopt) requires (!HasText<typename E::DescriptorType>) {
            auto& manager = renderTree.animationManager;
            auto& slot = node->animations[animation::Animatable::Color];
            simd_float4 initial = from.value_or(manager.color(slot, node->getColor()));

            slot = animation::AnimationState{manager.now, manager.now + duration, easing, initial};

            return color(target);
        }

        const std::string& text() const requires HasText<typename E::DescriptorType> {
            return descriptor().text;
        }

        Derived& text(const std::string& text) requires HasText<typename E::DescriptorType> {
            descriptor().text = text;
            markDirty(textDirtyBits());
            return self();
        }

        std::optional<std::string> font() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.font;
        }

        Derived& font(const std::string& fontPath) requires (!HasText<typename E::DescriptorType>) {
            node->shared.font = fontPath;
            markDirty(textDirtyBits());
            return self();
        }

        std::optional<Size> fontSize() const requires (!HasText<typename E::DescriptorType>) {
            return node->shared.fontSize;
        }

        Derived& fontSize(Size size) requires (!HasText<typename E::DescriptorType>) {
            node->shared.fontSize = size;
            markDirty(textDirtyBits());
            return self();
        }

        Derived& lineHeight(float multiplier) requires (!HasText<typename E::DescriptorType>) {
            node->shared.lineHeight = multiplier;
            markDirty(textDirtyBits());
            return self();
        }

        Derived& whiteSpace(WhiteSpace value) requires (!HasText<typename E::DescriptorType>) {
            node->shared.whiteSpace = value;
            markDirty(textDirtyBits());
            return self();
        }

        Derived& wordBreak(WordBreak value) requires (!HasText<typename E::DescriptorType>) {
            node->shared.wordBreak = value;
            markDirty(textDirtyBits());
            return self();
        }

        using ElemT = Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>;

    private:
        typename E::DescriptorType& descriptor() {
            auto* elem = static_cast<ElemT*>(node->element.get());
            return elem->element.getDescriptor();
        }

        const typename E::DescriptorType& descriptor() const {
            auto* elem = static_cast<ElemT*>(node->element.get());
            return elem->element.getDescriptor();
        }
    };

    template <ElementType E, typename P>
        requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
    struct EventNode : NodeMutation<EventNode<E,P>, E, P> {
        using Base = NodeMutation<EventNode<E,P>, E, P>;

        EventNode(RenderTree& tree, TreeNode* node):
            Base{tree, node}
        {}
    };

    template <ElementType E, typename P>
        requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
    struct NodeBuilder : NodeMutation<NodeBuilder<E,P>, E, P> {
        using Base = NodeMutation<NodeBuilder<E,P>, E, P>;

        UIContext& ctx;
        std::vector<NodeBuilder> children;

        NodeBuilder(UIContext& ctx, RenderTree& tree, E elem, P& proc):
            Base{tree, nullptr},
            ctx{ctx}
        {
            auto root = this->renderTree.getRoot();

            auto n = std::make_unique<TreeNode>(
                ctx, std::move(elem), proc
            );

            this->node = n.get();

            root->attach_child(std::move(n));
            this->renderTree.markDirty(root, Base::layoutDirtyBits() | DirtyBits::PaintOrder);
        }


        static TreeNode* reparent(TreeNode* newParent, TreeNode* child) {
            auto* oldParent = child->parent;
            auto& siblings = child->parent->children;
            auto it = std::find_if(siblings.begin(), siblings.end(), [&](auto& elem){
                return elem.get() == child;
            });

            if (it == siblings.end()) return nullptr;

            std::unique_ptr<TreeNode> moved = std::move(*it);
            siblings.erase(it);

            newParent->attach_child(std::move(moved));
            return oldParent;
        }

        template <typename... Children>
        NodeBuilder& operator()(Children&&... args) requires (!LeafElement<E>) {
            (([&] {
                auto* oldParent = reparent(this->node, args.treeNode());
                if (oldParent) {
                    this->renderTree.markDirty(oldParent, Base::layoutDirtyBits() | DirtyBits::PaintOrder);
                    this->renderTree.markDirty(this->node, Base::layoutDirtyBits() | DirtyBits::PaintOrder);
                }
            }()), ...);
            return *this;
        }


        using NodeBuilderEventHandler = std::function<void(EventNode<E,P>& node, Event& event)>;

        NodeBuilder<E,P>& addEventListener(EventType type, NodeBuilderEventHandler handler) {
            auto stableNode = this->node;
            auto* tree = &this->renderTree;

            auto func = [stableNode, tree, handler](Event& event){
                EventNode<E,P> eventNode{*tree, stableNode};
                handler(eventNode, event);
            };

            this->node->addEventListener(type, func);
            return *this;
        }

        TreeNode* treeNode() const {
            return this->node;
        }
    };

    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div();
    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> div(Size width, Size height, simd_float4 color);
    NodeBuilder<Div<DivStorage>, DivProcessor<DivStorage, DivUniforms>> span();
    NodeBuilder<Text<TextStorage>, TextProcessor<TextStorage, TextUniforms>> text(const std::string& text);
    NodeBuilder<Image<ImageStorage>, ImageProcessor<ImageStorage, ImageUniforms>> image(const std::string& path,
                    Size width = Size::px(0), Size height = Size::px(0));
    NodeBuilder<SVG<SVGStorage>, SVGProcessor<SVGStorage, SVGUniforms>> svg(const std::string& path,
                    Size width = Size::px(0), Size height = Size::px(0));
}
