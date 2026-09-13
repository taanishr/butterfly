#pragma once

#include "element.hpp"

namespace tree {
    struct RenderTree;

    using elements::Element;
    using elements::ElementBase;
    using elements::ElementType;
    using elements::ProcessorType;
    using layout::Atomized;
    using layout::ChainID;
    using layout::Constraints;
    using layout::LayoutResult;
    using layout::LineBox;
    using layout::LineFragment;
    using layout::Placed;
    using layout::PreLayoutResult;
    using runtime::Event;
    using runtime::EventType;
    using runtime::UIContext;
    using style::AlignContent;
    using style::AlignItems;
    using style::AlignSelf;
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
    using style::SharedDescriptor;
    using style::Size;

    struct TreeNode;

    enum class DirtyBits : uint32_t {
        None = 0,
        Measure = 1 << 0,
        Atomize = 1 << 1,
        Layout = 1 << 2,
        PostLayout = 1 << 3,
        Place = 1 << 4,
        Finalize = 1 << 5,
        PaintOrder = 1 << 6,
    };

    constexpr DirtyBits operator|(DirtyBits a, DirtyBits b) {
        return static_cast<DirtyBits>(std::to_underlying(a) | std::to_underlying(b));
    }

    constexpr DirtyBits operator&(DirtyBits a, DirtyBits b) {
        return static_cast<DirtyBits>(std::to_underlying(a) & std::to_underlying(b));
    }

    constexpr DirtyBits operator~(DirtyBits bits) {
        return static_cast<DirtyBits>(~std::to_underlying(bits));
    }

    inline DirtyBits& operator|=(DirtyBits& a, DirtyBits b) {
        a = a | b;
        return a;
    }

    inline DirtyBits& operator&=(DirtyBits& a, DirtyBits b) {
        a = a & b;
        return a;
    }

    constexpr bool hasDirty(DirtyBits value, DirtyBits bits) {
        return (static_cast<uint32_t>(value & bits) != 0);
    }

    constexpr DirtyBits allPhaseDirtyBits() {
        return DirtyBits::Measure | DirtyBits::Atomize | DirtyBits::Layout |
            DirtyBits::PostLayout | DirtyBits::Place | DirtyBits::Finalize | DirtyBits::PaintOrder;
    }

    struct ConstraintsKey {
        std::size_t value{};

        bool operator==(const ConstraintsKey& other) const {
            return value == other.value;
        }
    };

    struct CollapsedChain {
        ChainID id;
        TreeNode* root;
        Size intent;
        int depth;
    };
    
    using EventHandler = std::function<void(Event&)>;

    struct TreeNode {
        template<ElementType E, typename P>
            requires ProcessorType<P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>
        TreeNode(UIContext& ctx, E&& elem, P& processor)
            : element(std::make_unique<Element<E,P, typename E::StorageType, typename E::DescriptorType, typename E::UniformsType>>(ctx, std::move(elem), processor)),
            parent(nullptr),
            id(nextId++),
            localZIndex{0},
            globalZIndex{0},
            paintPreorderIndex{0},
            paintPostorderIndex{0}
        {}

        ~TreeNode() {
            instrumentation::removeNode(id);
        }

        void addEventListener(EventType type, EventHandler handler) {
            eventHandlers[type].push_back(std::move(handler));
        }

        TreeNode* dispatch(Event& event) {
            auto it = eventHandlers.find(event.type);
            if (it != eventHandlers.end()) {
                for (auto& handler : it->second) {
                    if (event.propagationStopped) break;
                    handler(event);
                }
            }

            if (event.type == EventType::ScrollWheel && shared.overflow == Overflow::Scroll) {
                auto& scroll = event.get<EventType::ScrollWheel>();
                float maxScrollX = 0.0f;
                float maxScrollY = 0.0f;
                if (layout.has_value()) {
                    maxScrollX = std::max(0.0f, scrollContentSize.x - scrollViewportSize.x);
                    maxScrollY = std::max(0.0f, scrollContentSize.y - scrollViewportSize.y);
                }
                auto oldScrollOffset = scrollOffset;

                scrollOffset.x = std::clamp(scrollOffset.x - scroll.dx, 0.0f, maxScrollX);
                scrollOffset.y = std::clamp(scrollOffset.y - scroll.dy, 0.0f, maxScrollY);
                event.stopPropagation();
                if (oldScrollOffset.x != scrollOffset.x || oldScrollOffset.y != scrollOffset.y) {
                    return this;
                }
            }

            if (!event.propagationStopped && parent) {
                return parent->dispatch(event);
            }

            return nullptr;
        }

        void calculateGlobalZIndex(uint64_t parentGlobal) {
            globalZIndex = std::add_sat(parentGlobal, localZIndex);
            
            for (auto& child : children) {
                child->calculateGlobalZIndex(globalZIndex);
            }
        }
        
        void attach_child(std::unique_ptr<TreeNode>&& child) {
            if (!child) return;
            child->parent = this;
            children.push_back(std::move(child));
        }

        bool contains(simd_float2 point) const {
            if (shared.pointerEvents == PointerEvents::None) return false;
            if (!layout.has_value()) return false;
            
            bool withinBounds = std::visit([&](const auto& state) {
                const auto& box = state.computedBox;

                if (point.x < box.x || point.x > box.x + box.width ||
                    point.y < box.y || point.y > box.y + box.height) {
                    return false;
                }

                for (const auto& clip : state.clipUniforms) {
                    if (rounded_rect_sdf(point - clip.rectCenter, clip.halfExtent, clip.cornerRadius) > 0.0f) {
                        return false;
                    }
                }

                return true;
            }, layout->layout);

            if (!withinBounds) return false;

            return element->preciseHitTest(point, layout->layout, finalized);
        }

        Position getPosition() const { return shared.position; }
        Display getDisplay() const { return computedDisplay.value_or(shared.display); }
        Size getMarginTop() const { return shared.marginTop.value_or(shared.margin); }
        Size getMarginBottom() const { return shared.marginBottom.value_or(shared.margin); }
        Size getMarginLeft() const { return shared.marginLeft.value_or(shared.margin); }
        Size getMarginRight() const { return shared.marginRight.value_or(shared.margin); }
        Size getMargin() const { return shared.margin; }
        std::optional<Size> getPaddingTop() const { return shared.paddingTop; }
        std::optional<Size> getPaddingBottom() const { return shared.paddingBottom; }
        Size getFlexGrow() const { return shared.flexGrow; }
        Size getFlexShrink() const { return shared.flexShrink; }
        FlexDirection getFlexDirection() const { return shared.flexDirection; }
        JustifyContent getJustifyContent() const { return shared.justifyContent; }
        AlignItems getAlignItems() const { return shared.alignItems; }
        Size getFlexGap() const { return shared.flexGap; }
        FlexWrap getFlexWrap() const { return shared.flexWrap; }
        AlignContent getAlignContent() const { return shared.alignContent; }
        AlignSelf getAlignSelf() const { return shared.alignSelf; }
        JustifyItems getJustifyItems() const { return shared.justifyItems; }
        JustifySelf getJustifySelf() const { return shared.justifySelf; }
        const std::vector<Size>& getGridTemplateColumns() const { return shared.gridTemplateColumns; }
        const std::vector<Size>& getGridTemplateRows() const { return shared.gridTemplateRows; }
        Size getGridColumnGap() const { return shared.gridColumnGap; }
        Size getGridRowGap() const { return shared.gridRowGap; }
        GridPlacement getGridPlacement() const { return shared.gridPlacement; }

        
        std::unique_ptr<ElementBase> element;
        TreeNode* parent = nullptr;
        std::vector<std::unique_ptr<TreeNode>> children;
        uint64_t id;
        uint64_t localZIndex;
        uint64_t globalZIndex;
        uint64_t paintPreorderIndex;
        uint64_t paintPostorderIndex;

        std::optional<Atomized> atomized;
        std::optional<PreLayoutResult> preLayout;
        std::optional<LayoutResult> layout;
        std::optional<bidi::TextBidiInput> textBidiInput;
        std::optional<Placed> placed;
        std::unordered_map<EventType, std::vector<EventHandler>> eventHandlers;
        std::any finalized;
        simd_float2 globalOffset {0.0f, 0.0f};
        simd_float2 scrollOffset {0.0f, 0.0f};
        simd_float2 scrollContentSize {0.0f, 0.0f};
        simd_float2 scrollViewportSize {0.0f, 0.0f};
        SharedDescriptor shared;
        std::optional<Display> computedDisplay;
        DirtyBits dirtySelf{~DirtyBits::None};
        DirtyBits dirtySubtree{~DirtyBits::None};
        std::optional<ConstraintsKey> constraintsKey;

    private:
        static uint64_t nextId;
    };

    std::vector<TreeNode*> collectAllNodes(TreeNode* root);
    std::optional<std::string> getText(TreeNode* node);
    std::optional<style::WhiteSpace> getWhiteSpace(TreeNode* node);
    Result<std::vector<std::optional<bidi::TextBidiInput>>>
    prepareChildBidiInputs(
        TreeNode* parent,
        layout::Direction baseDirection
    );

    void precomputeMargins(RenderTree& tree, TreeNode* node, Constraints& constraints, std::unordered_map<ChainID, CollapsedChain>& collapsedChainMap);
    
    // full blown inline context
    std::shared_ptr<layout::InlineFormattingContext> buildInlineBoxes(TreeNode* node, const InlineSizingInput& sizing);

    // inline context calculated for a single child, independently of other siblings
    layout::InlineFormattingInput buildIsolatedInlineBoxes(TreeNode* node, const InlineSizingInput& sizing);

}

namespace std {
    template<>
    struct hash<tree::ConstraintsKey> {
        size_t operator()(const tree::ConstraintsKey& key) const noexcept {
            return key.value;
        }
    };
}
