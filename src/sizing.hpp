#pragma once
#include <expected>
#include <optional>
#include <variant>

namespace style {
    enum class SizeError {
        Auto,
        IndefiniteBasis,
        FractionRequiresContext,
        ContentDependent
    };

    enum class Unit {
        Px,
        Percent,
        Auto,
        Pt,
        Fr,
        MinContent,
        MaxContent,
        FitContent
    };

    struct Size {
        float value;
        Unit unit;

        Size():
            value{0},
            unit{Unit::Px}
        {};

        static Size px(float v)       { return {v, Unit::Px}; }
        static Size pt(float v)       { return {v, Unit::Pt}; }
        static Size percent(float v)  {
            if (v < 0.0)
                return {0.0, Unit::Percent};
            else if (v > 1.0)
                return {1.0, Unit::Percent};
            else
                return {v, Unit::Percent};
        }
        static Size autoSize()        { return {0.0f, Unit::Auto}; }
        static Size fr(float v)       { return {v, Unit::Fr}; }
        static Size minContent()      { return {0.0f, Unit::MinContent}; }
        static Size maxContent()      { return {0.0f, Unit::MaxContent}; }
        static Size fitContent()      { return {0.0f, Unit::FitContent}; }

        bool isAuto() const { return unit == Unit::Auto; }
        bool isFr() const { return unit == Unit::Fr; }
        bool isContentDependent() const {
            return unit == Unit::MinContent
                || unit == Unit::MaxContent
                || unit == Unit::FitContent;
        }

        std::expected<float, SizeError> resolve(const Size& basis) const {
            switch (unit) {
                case Unit::Px:
                    return value;
                case Unit::Auto:
                    return std::unexpected(SizeError::Auto);
                case Unit::Pt:
                    return value;
                case Unit::Fr:
                    return std::unexpected(SizeError::FractionRequiresContext);
                case Unit::Percent:
                    if (basis.unit == Unit::Px || basis.unit == Unit::Pt) {
                        return value * basis.value;
                    }
                    return std::unexpected(SizeError::IndefiniteBasis);
                case Unit::MinContent:
                case Unit::MaxContent:
                case Unit::FitContent:
                    return std::unexpected(SizeError::ContentDependent);
            }
        }

        float resolveOr(const Size& basis, float defaultVal = 0.0f) const {
            auto resolved = resolve(basis);
            return resolved.value_or(defaultVal);
        }

        private:
            constexpr Size(float v, Unit u) : value(v), unit(u) {}
    };
}

using SizeState = std::variant<std::monostate, style::Size, float, style::SizeError>;

struct SizePair {
    SizeState width;
    SizeState height;
};

namespace layout {
    struct IntrinsicSizes {
        style::Size minContent;
        style::Size maxContent;
    };
}
