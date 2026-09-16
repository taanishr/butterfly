#pragma once

#include "sdf_helpers.hpp"
#include "metal_imports.hpp"
#include "fragment.hpp"
#include "layout/layout.hpp"
#include "layout/sizing.hpp"
#include <concepts>
#include <any>
#include <cstdint>
#include <algorithm>
#include <unordered_map>
#include "frame_info.hpp"
#include "instrumentation.hpp"
#include <optional>
#include <print>
#include <simd/vector_types.h>
#include <string_view>
#include "parallel.hpp"
#include "events.hpp"
#include "printers.hpp"
#include <numeric> 

namespace elements {
    using layout::Atomized;
    using layout::Constraints;
    using layout::Finalized;
    using layout::LayoutState;
    using layout::Placed;
    using runtime::HitTestContext;
    using runtime::UIContext;
    using style::SharedDescriptor;

    constexpr bool isTextWhitespace(char32_t codepoint) noexcept {
        return codepoint == U' '  ||
               codepoint == U'\t' ||
               codepoint == U'\r' ||
               codepoint == U'\n';
    }

    enum class RequestTarget {
        Descriptor,
        TextShaping,
    };

    template<typename E>
    concept ElementType = requires(E& e, RequestTarget target, std::any& payload) {
        e.getDescriptor();
        e.getFragment();
        { e.request(target, payload) } -> std::same_as<std::any>;

        typename E::StorageType;
        typename E::DescriptorType;
        typename E::UniformsType;
    } && DescriptorType<typename E::DescriptorType>;

    template<typename P, typename S, typename D, typename U>
    concept ProcessorType = requires(
        P& proc,
        Fragment<S>& fragment,
        Constraints& constraints,
        SharedDescriptor& shared,
        D& desc,
        Atomized& atomized,
        const SizeResult& sizeResult,
        Placed& placed,
        Finalized<U>& finalized,
        layout::BlockState blockState,
        MTL::RenderCommandEncoder* encoder
    ) {
        { proc.atomize(fragment, constraints, shared, desc) } -> std::same_as<Atomized>;
        { proc.layout(fragment, constraints, shared, desc, atomized, sizeResult) } -> std::same_as<LayoutState>;

        { proc.postLayout(fragment, constraints, shared, desc, atomized, blockState) } -> std::same_as<Atomized>;

        { proc.place(fragment, constraints, shared, desc, atomized, blockState) } -> std::same_as<Placed>;

        { proc.finalize(fragment, constraints, shared, desc, atomized, blockState, placed) } -> std::same_as<Finalized<U>>;
        { proc.setupHitTestFunction() } -> std::same_as<std::function<bool(HitTestContext<U>&, simd_float2)>>;
        proc.encode(encoder, fragment, finalized);
    };

    struct ElementBase {
        virtual Atomized atomize(Constraints& constraints, SharedDescriptor& shared) = 0;
        virtual LayoutState layout(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, const SizeResult& sizeResult) = 0;
        virtual Atomized postLayout(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, LayoutState& layout) = 0;
        virtual Placed place(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, LayoutState& layout) = 0;
        virtual std::any finalize(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, LayoutState& layout, Placed& placed) = 0;
        virtual std::any request(RequestTarget target, std::any& payload) = 0;
        virtual void encode(MTL::RenderCommandEncoder* encoder, std::any& finalized) = 0;
        virtual std::string_view elementTypeName() const = 0;
        virtual bool isInline() const {
            return false;
        }
        virtual bool isReplaced() const {
            return false;
        }
        virtual bool preciseHitTest(simd_float2 point, const LayoutState& layout, const std::any& finalized) {
            return true;
        }

        virtual ~ElementBase() = 0;
    };

    template <ElementType E, typename P, typename S, typename D, typename U>
        requires ProcessorType<P, S, D, U>
    struct Element : ElementBase {
        Element(UIContext& ctx, E&& elem, P& proc):
            element{std::move(elem)}, processor{proc}
        {
            hitTestFunction = processor.setupHitTestFunction();
        }

        Atomized atomize(Constraints& constraints, SharedDescriptor& shared) override {
            return processor.atomize(element.getFragment(), constraints, shared, element.getDescriptor());
        }

        LayoutState layout(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, const SizeResult& sizeResult) override {
            return processor.layout(element.getFragment(), constraints, shared, element.getDescriptor(), atomized, sizeResult);
        }

        Atomized postLayout(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, LayoutState& layout) override {
            return std::visit([&](auto& state) {
                return processor.postLayout(element.getFragment(), constraints, shared, element.getDescriptor(), atomized, state);
            }, layout);
        }

        Placed place(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, LayoutState& layout) override {
            return std::visit([&](auto& state) {
                return processor.place(element.getFragment(), constraints, shared, element.getDescriptor(), atomized, state);
            }, layout);
        }

        std::any finalize(Constraints& constraints, SharedDescriptor& shared, Atomized& atomized, LayoutState& layout, Placed& placed) override {
            auto finalized = std::visit([&](auto& state) {
                return processor.finalize(element.getFragment(), constraints, shared, element.getDescriptor(), atomized, state, placed);
            }, layout);
            auto finalizedErased = finalized;
            return finalizedErased;
        }

        std::any request(RequestTarget target, std::any& payload) override {
            return element.request(target, payload);
        };

        void encode(MTL::RenderCommandEncoder* encoder, std::any& finalizedErased) override {
            auto finalized = std::any_cast<Finalized<typename E::UniformsType>>(finalizedErased);
            return processor.encode(encoder, element.getFragment(), finalized);
        }

        std::string_view elementTypeName() const override {
            if constexpr (requires { E::elementName; }) {
                return E::elementName;
            }

            return "Unknown";
        }

        bool isInline() const override {
            if constexpr (requires(const E& value) { value.isInline(); }) {
                return element.isInline();
            }
            return false;
        }

        bool isReplaced() const override {
            if constexpr (requires(const E& value) { value.isReplaced(); }) {
                return element.isReplaced();
            }
            return false;
        }

        bool preciseHitTest(simd_float2 point, const LayoutState& layout, const std::any& finalized) override {
            if (hitTestFunction) {
                HitTestContext<U> ctx {
                    .finalized = std::any_cast<Finalized<U>>(finalized),
                    .layout = layout
                };

                return hitTestFunction(ctx, point);
            }

            return true;
        }

        ~Element() override {};

        std::function<bool(HitTestContext<U>& context, simd_float2 testPoint)> hitTestFunction;
        E element;
        P& processor;
    };
}
