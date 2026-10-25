#pragma once

#include "layout/sizing.hpp"
#include <simd/simd.h>
#include <algorithm>
#include <cstddef>
#include <optional>
#include <variant>

namespace animation {
    enum class Easing {
        Linear,
        EaseInQuad,
        EaseOutQuad,
        EaseInOutQuad,
    };

    namespace Animatable {
        constexpr size_t Width = 0;
        constexpr size_t Height = 1;
        constexpr size_t Translate = 2;
        constexpr size_t Opacity = 3;
        constexpr size_t BackgroundColor = 4;
        constexpr size_t Color = 5;
        constexpr size_t Count = 6;
    }

    struct AnimationState {
        double start;
        double end;
        Easing easing;
        std::variant<style::Size, float, simd_float2, simd_float4> initial;
    };

    struct AnimationManager {
        double now{0.0};

        void advance(double frameTime) {
            now = frameTime;
        }

        bool nextAnimationStickyState(const AnimationState& state) const {
            return now < state.end;
        }

        style::Size width(const std::optional<AnimationState>& state, const style::Size& target) const {
            if (!state) {
                return target;
            }

            const auto& initial = std::get<style::Size>(state->initial);
            if (initial.unit != target.unit) {
                return target;
            }

            float value = initial.value + (target.value - initial.value) * progress(*state);

            switch (target.unit) {
                case style::Unit::Px:
                    return style::Size::px(value);
                case style::Unit::Pt:
                    return style::Size::pt(value);
                case style::Unit::Percent:
                    return style::Size::percent(value);
                case style::Unit::Fr:
                    return style::Size::fr(value);
                case style::Unit::Auto:
                case style::Unit::MinContent:
                case style::Unit::MaxContent:
                case style::Unit::FitContent:
                    return target;
            }

            return target;
        }

        style::Size height(const std::optional<AnimationState>& state, const style::Size& target) const {
            if (!state) {
                return target;
            }

            const auto& initial = std::get<style::Size>(state->initial);
            if (initial.unit != target.unit) {
                return target;
            }

            float value = initial.value + (target.value - initial.value) * progress(*state);

            switch (target.unit) {
                case style::Unit::Px:
                    return style::Size::px(value);
                case style::Unit::Pt:
                    return style::Size::pt(value);
                case style::Unit::Percent:
                    return style::Size::percent(value);
                case style::Unit::Fr:
                    return style::Size::fr(value);
                case style::Unit::Auto:
                case style::Unit::MinContent:
                case style::Unit::MaxContent:
                case style::Unit::FitContent:
                    return target;
            }

            return target;
        }

        simd_float3x3 transform(const std::optional<AnimationState>& state, simd_float3x3 target) const {
            if (!state) {
                return target;
            }

            simd_float2 initial = std::get<simd_float2>(state->initial);
            simd_float2 translate = target.columns[2].xy;
            translate = initial + (translate - initial) * progress(*state);
            target.columns[2] = {translate.x, translate.y, 1.0f};
            return target;
        }

        float opacity(const std::optional<AnimationState>& state, float target) const {
            if (!state) {
                return target;
            }

            float initial = std::get<float>(state->initial);
            return initial + (target - initial) * progress(*state);
        }

        simd_float4 color(const std::optional<AnimationState>& state, simd_float4 target) const {
            if (!state) {
                return target;
            }

            simd_float4 initial = std::get<simd_float4>(state->initial);
            return initial + (target - initial) * progress(*state);
        }

    private:
        float progress(const AnimationState& state) const {
            if (state.end <= state.start) {
                return 1.0f;
            }

            float elapsed = static_cast<float>(std::clamp((now - state.start) / (state.end - state.start), 0.0, 1.0));

            switch (state.easing) {
                case Easing::Linear:
                    return elapsed;
                case Easing::EaseInQuad:
                    return elapsed * elapsed;
                case Easing::EaseOutQuad:
                    return elapsed * (2.0f - elapsed);
                case Easing::EaseInOutQuad:
                    return elapsed < 0.5f
                        ? 2.0f * elapsed * elapsed
                        : -1.0f + (4.0f - 2.0f * elapsed) * elapsed;
            }

            return elapsed;
        }
    };
}
