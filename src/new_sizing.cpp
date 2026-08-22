#include "new_sizing.hpp"
#include "element.hpp"
#include "new_arch.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <MacTypes.h>
#include <algorithm>
#include <format>
#include <optional>
#include <print>
#include <sys/kauth.h>
#include <variant>

template <>
struct std::formatter<style::SizeError> : std::formatter<std::string_view> {
    auto format(style::SizeError error, format_context& ctx) const {
        std::string_view name;
        switch (error) {
            case style::SizeError::Auto: name = "Auto"; break;
            case style::SizeError::IndefiniteBasis: name = "IndefiniteBasis"; break;
            case style::SizeError::FractionRequiresContext: name = "FractionRequiresContext"; break;
            case style::SizeError::ContentDependent: name = "ContentDependent"; break;
        }
        return std::formatter<std::string_view>::format(name, ctx);
    }
};

template <>
struct std::formatter<style::Size> : std::formatter<std::string_view> {
    auto format(const style::Size& size, format_context& ctx) const {
        std::string_view unit;
        switch (size.unit) {
            case style::Unit::Px: unit = "px"; break;
            case style::Unit::Percent: unit = "%"; break;
            case style::Unit::Auto: unit = "auto"; break;
            case style::Unit::Pt: unit = "pt"; break;
            case style::Unit::Fr: unit = "fr"; break;
            case style::Unit::MinContent: unit = "min-content"; break;
            case style::Unit::MaxContent: unit = "max-content"; break;
            case style::Unit::FitContent: unit = "fit-content"; break;
        }

        if (size.unit == style::Unit::Auto || size.isContentDependent())
            return std::format_to(ctx.out(), "{}", unit);
        return std::format_to(ctx.out(), "{}{}", size.value, unit);
    }
};

template <>
struct std::formatter<SizeState> : std::formatter<std::string_view> {
    auto format(const SizeState& size, format_context& ctx) const {
        return std::visit(Overloaded{
            [&](std::monostate) { return std::format_to(ctx.out(), "unset"); },
            [&](const auto& value) { return std::format_to(ctx.out(), "{}", value); }
        }, size);
    }
};

template <>
struct std::formatter<std::expected<float, style::SizeError>> : std::formatter<std::string_view> {
    auto format(const std::expected<float, style::SizeError>& size, format_context& ctx) const {
        if (size)
            return std::format_to(ctx.out(), "{}", *size);
        return std::format_to(ctx.out(), "{}", size.error());
    }
};


// circular problem:
// constraints -> determine request
// but intrinsic SizeSpec measurement -> requires constraints that largely mirror existing ones?
// so define a clear separation.
// thats fine, bc both of them are separate 

auto calculateSize(const SizeState& size, const SizeState& available) -> SizeState {
    return std::visit(Overloaded{
        [&](const style::Size& size) -> SizeState {
            switch (size.unit) {
                case style::Unit::Px:
                case style::Unit::Pt:
                    return size.value;
                case style::Unit::Auto:
                    return style::SizeError::Auto;
                case style::Unit::Fr:
                    return style::SizeError::FractionRequiresContext;
                case style::Unit::MinContent:
                case style::Unit::MaxContent:
                case style::Unit::FitContent:
                    return style::SizeError::ContentDependent;
                case style::Unit::Percent: {
                    SizeState calculatedAvailable = calculateSize(available, std::monostate{});

                    return std::visit(Overloaded{
                        [&](float availableSize) -> SizeState {
                            return size.value * availableSize;
                        },
                        [](std::monostate) -> SizeState {
                            return style::SizeError::IndefiniteBasis;
                        },
                        [](style::SizeError error) -> SizeState {
                            switch (error) {
                                case style::SizeError::ContentDependent:
                                case style::SizeError::FractionRequiresContext:
                                    return error;
                                case style::SizeError::Auto:
                                case style::SizeError::IndefiniteBasis:
                                    return style::SizeError::IndefiniteBasis;
                            }

                            return style::SizeError::IndefiniteBasis;
                        },
                        [](const style::Size&) -> SizeState {
                            return style::SizeError::IndefiniteBasis;
                        }
                    }, calculatedAvailable);
                }
            }

            return std::monostate{};
        },
        [](const auto& other) -> SizeState {
            return SizeState{other};
        }
    }, size);
}

// IMPL

// given a set of rules, resolve the width and height
// specified
// resolve width
//   - specified or intrinsic
//   - automatic from opposing insets
//   - automatic using available size
//   - automatic using content size
// resolve height
//   - specified or intrinsic
//   - automatic from opposing insets
//   - automatic using available size
//   - automatic using content size
// needs to be imbued with ctx
auto resolveWidth(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState {
    // run size through a calculate size pass (maybe avail too)
    SizeState resolved = calculateSize(size, req.available.width);

    // if our resulting size is a float
    const auto* error = std::get_if<SizeError>(&resolved);
    
    // return specified right away
    if (!error) {
        return resolved;
    }

    if (*error == SizeError::ContentDependent) {
        if (!intrinsic) {
            return resolved;
        }

        return resolveIntrinsicWidth(size, *intrinsic, req);
    }

    // if our resulting size is automatic
    if (*error == SizeError::Auto) {
        // are we content sizing?
        if (req.automaticWidth == AutomaticSizing::UseContent) {
            return calculateSize(req.content.width, req.available.width);
        }
        
        // are we avail sizing?
        // first, refine the available size
        SizeState available = calculateSize(req.available.width, std::monostate{});
        const auto* availableWidth = std::get_if<float>(&available);

        if (!availableWidth) {
            return available;
        }

        SizeState paddingLeft = calculateSize(req.paddingLeft, req.available.width);
        const auto* resolvedPaddingLeft = std::get_if<float>(&paddingLeft);

        if (!resolvedPaddingLeft) {
            return paddingLeft;
        }

        SizeState paddingRight = calculateSize(req.paddingRight, req.available.width);
        const auto* resolvedPaddingRight = std::get_if<float>(&paddingRight);

        if (!resolvedPaddingRight) {
            return paddingRight;
        }

        SizeState borderWidth = calculateSize(req.borderWidth, req.available.width);
        const auto* resolvedBorderWidth = std::get_if<float>(&borderWidth);

        if (!resolvedBorderWidth) {
            return borderWidth;
        }

        float automaticWidth = *availableWidth
            - req.margins.left
            - req.margins.right
            - *resolvedPaddingLeft
            - *resolvedPaddingRight
            - 2.0f * *resolvedBorderWidth;

        // then, branch from here
        // are we *out of flow* and do we have insets?
        bool outOfFlow = req.position == style::Position::Absolute || req.position == style::Position::Fixed;
        bool opposingInsets = req.left.has_value() && req.right.has_value();

        if (outOfFlow && opposingInsets) {
            SizeState left = calculateSize(*req.left, req.available.width);
            const auto* resolvedLeft = std::get_if<float>(&left);

            if (!resolvedLeft) {
                return left;
            }

            SizeState right = calculateSize(*req.right, req.available.width);
            const auto* resolvedRight = std::get_if<float>(&right);

            if (!resolvedRight) {
                return right;
            }

            return automaticWidth - *resolvedLeft - *resolvedRight;
        }

        // otherwise: just stretch
        return automaticWidth;
    }

    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
    return resolved;
}

auto resolveHeight(const SizeState& size, const SizeRequest* req) -> SizeState {
    // run size through a calculate size pass (maybe avail too)

    // if our resulting size is a float
        // return specified right away

    /* open question: at this point, should we figure out the intrinsic sizes? 
        Should this method even resolve intrinsic sizes
        That is unclear
    */

    // if our resulting size is automatic
        // are we content sizing?
            // should we resolve intrinsic sizes here? or before?
            // figure out the necessary intrinsic sizes
            // and content size accordingly (min, max, or fit)
        // are we avail sizing:
            // are we *out of flow* and do we have insets?
                // then use that against the available
            // else
                // just stretch

    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
}

// these ONLY exist because of different auto behavior fo min/max width and height
auto resolveMinWidth(const SizeState& size) -> SizeState {
    // run size through a calculate size pass (maybe avail too)

    // if our resulting size is a float
        // return specified right away

    // if our resulting size is automatic (differs from above):
        // usually, floor it to 0 (the automatic minimum)

    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
}

auto resolveMaxWidth(const SizeState& size) -> SizeState {
    // run size through a calculate size pass (maybe avail too)

    // if our resulting size is a float
        // return specified right away

    // there is no automatic branch; i do not want to throw an error.
    //  maybe no-op this? Dont do anything?
    
    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
}

auto resolveMinHeight(const SizeState& size) -> SizeState {
    // run size through a calculate size pass (maybe avail too)

    // if our resulting size is a float
        // return specified right away

    // if our resulting size is automatic (differs from above):
        // usually, floor it to 0 (the automatic minimum)

    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
}

auto resolveMaxHeight(const SizeState& size) -> SizeState  {
    // run size through a calculate size pass (maybe avail too)

    // if our resulting size is a float
        // return specified right away

    // there is no automatic branch; i do not want to throw an error.
    //  maybe no-op this? Dont do anything?
    
    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
}

// handles intrinsic cases
// intrinsic width (min, max, fit content)
// intrinsic height (min, max, fit content)

// the anti revolves around these two ops
// intrinsic height given set width (previously resolved)
// unknown/skip: intrinsic width based on resolved height: unlikely / nonexistent
// to start, the anti wil be ignored for resolveIntrinsicWidth
// this should likely not return a size state, but probably a min and max size (intrinsic result type)
// IntrinsicResult? (intrinsic sizes as is is not satisfactory and relies on old Size, which is more of a description than a proper intermediate)

auto measureIntrinsicWidth(const SizeState& size, const SizeState& antiSize) -> IntrinsicResult {
    // antiSize: not used here

    // first; create a new request (or have the recursive tree func do this)
    // this new request should set resolvingIntrinsicWidth = true
    // afterwards, establish the recursive call
    /*
        BIG TODO: (not yet done, and kind of only semi-related to this function)
        create an abstraction to make the inline text breaking algorithm less dependent
        on the leakiness of size; maybe we should create a separate
        InlineContext? that the buildInlineBoxes take in
        and that is generated by a function here
    */
    // return both the min and max intrinsic size
}

auto measureIntrinsicHeight(const SizeState& size, const SizeState& antiSize) -> IntrinsicResult {
    // antisize: USED HERE

    // first; create a new request (or have the recursive tree func do this)
    // this new request should set resolvingIntrinsicHeight = true
    
    // if an antiSize has been resolved and provided via antisize, the request SHOULD note this
    // establish the recursive call; make sure it establishes the specified antiSize correcttly

    // return both the min and max intrinsic size
}

// determine based on min/max/fit content and provided sizes
auto resolveIntrinsicWidth(const SizeState& size, const IntrinsicResult& intrinsic, SizeRequest& req) -> SizeState {
    // fairly simple
    // take the min and max, and according to the req:
        // req requires min
            // return min
        // req requires max
            // return max
        // req requires fit content
            // clampSize with min and max set

    /*
        Open question: should we return a monostate if neither min, max or fit? Or default to fit?
    */
}
auto resolveIntrinsicHeight(const SizeState& min, const SizeState& max, const SizeRequest* req) -> SizeState {
    // fairly simple
    // take the min and max, and according to the req:
        // req requires min
            // return min
        // req requires max
            // return max
        // req requires fit content
            // clampSize with min and max set

    /*
        Open question: should we return a monostate if neither min, max or fit? Or default to fit?
    */
}


// clamp a size
// i went through two design ideas:
// clampSizeAgainstMin & clampSizeAgainstMax separation 
// i decided on a unified version, where you pass monostate if one does not exist
// this will probably end up in a few nested variants
// this may cause issues:
// issue 1: what if min > max? min wins
// issue 2: does this create an implied fit content operation? basically, if neither is monostate
auto clampSize(const SizeState& size, const SizeState& min, const SizeState& max) -> SizeState {
    // invariant: min/max have already been resolved
    // should we check that they have? Or is that a waste of time?
    // I lean towards the later

    // case 1: min and max defined
        // apply max
        // then apply min
        // return new size
    // case 2: min defined:
        // apply against min
        // return
    // case 3: max defined:
        // apply against max
        // return max
    // case 4: neither defined
        // to ensure case coverage
        // just no op and return exactly the asme thing
        // albeit i dont really like this because its wasted memory
        // allocating a new object

    const auto* resolvedSize = std::get_if<float>(&size);
    
    if (!resolvedSize) {
        return size;
    }

    float clamped = *resolvedSize;

    const auto* resolvedMax = std::get_if<float>(&max);
    if (resolvedMax) {
        clamped = std::min(clamped, *resolvedMax);
    }

    const auto* resolvedMin = std::get_if<float>(&min);
    if (resolvedMin) {
        clamped = std::max(clamped, *resolvedMin);
    }

    return clamped;
}   

// this will also probably end up as a massive function switching between possibilites
// no height, no width
// height, no width
// width, no height
// height, width
auto transferAspectRatio(const SizePair& pair, float ratio) -> SizePair {
    // first: check ratio
    // if less than or equal to 0.0, return the existing size pair
    // that being said, this is overly defensive and just should not happen
    if (ratio <= 0.0f) {
        return pair;
    }

    // case 1: width and height
        // return same size pair
    // case 2: width, no height
        // transfer width onto height via ratio
    // case 3: no widht, height:
        // transfer height onto width via ratio
    // case 4: no width and no height
        // no op, return same size pair

    const auto* width = std::get_if<float>(&pair.width);
    const auto* height = std::get_if<float>(&pair.height);

    SizePair result = pair;

    if (width && !height) {
        result.height = *width / ratio;
    } else if (height && !width) {
        result.width = *height * ratio;
    }

    return result;
}


auto evaluateSize(
    tree::RenderTree& tree,
    tree::TreeNode* node,
    const FrameInfo& frameInfo,
    layout::Constraints constraints,
    layout::Measured measured,
    SizeRequest req
) -> SizeResult
{
    const auto& requestedWidth = std::holds_alternative<std::monostate>(req.override.width)
        ? req.specified.width
        : req.override.width;
    const auto& requestedHeight = std::holds_alternative<std::monostate>(req.override.height)
        ? req.specified.height
        : req.override.height;

    SizePair size {
        .width = calculateSize(requestedWidth, req.available.width),
        .height = calculateSize(requestedHeight, req.available.height),
    };

    SizePair minimum {
        .width = calculateSize(req.minimum.width, req.available.width),
        .height = calculateSize(req.minimum.height, req.available.height),
    };
    
    SizePair maximum {
        .width = calculateSize(req.maximum.width, req.available.width),
        .height = calculateSize(req.maximum.height, req.available.height),
    };

    SizePair content {
        .width = calculateSize(req.content.width, req.available.width),
        .height = calculateSize(req.content.height, req.content.width)
    };
    
    // handle intrinsic sizes
    // width intrinsic sizes
    auto minWidthError = std::get_if<SizeError>(&minimum.width);
    auto maxWidthError = std::get_if<SizeError>(&maximum.width);
    auto widthError = std::get_if<SizeError>(&size.width);

    auto minWidthIntrinsicError = minWidthError && *minWidthError == SizeError::ContentDependent;
    auto maxWidthIntrinsicError = maxWidthError && *maxWidthError == SizeError::ContentDependent;
    auto widthIntrinsicError = widthError && *widthError == SizeError::ContentDependent;

    std::optional<layout::IntrinsicSizes> widthIntrinsicSizes;
    if (constraints.intrinsicSizesAxis != layout::Axis::Width && (minWidthIntrinsicError || maxWidthIntrinsicError || widthIntrinsicError)) {
        widthIntrinsicSizes = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, layout::Axis::Width);
    }
    
    // height intrinsic sizes
    auto minHeightError = std::get_if<SizeError>(&minimum.height);
    auto maxHeightError = std::get_if<SizeError>(&maximum.height);
    auto heightError = std::get_if<SizeError>(&size.height);

    auto minHeightIntrinsicError = minHeightError && *minHeightError == SizeError::ContentDependent;
    auto maxHeightIntrinsicError = maxHeightError && *maxHeightError == SizeError::ContentDependent;
    auto heightIntrinsicError = heightError && *heightError == SizeError::ContentDependent;

    std::optional<layout::IntrinsicSizes> heightIntrinsicSizes;
    if (constraints.intrinsicSizesAxis != layout::Axis::Height && (minHeightIntrinsicError || maxHeightIntrinsicError || heightIntrinsicError)) {
        heightIntrinsicSizes = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, layout::Axis::Height);
    }


    return SizeResult {
        .size = size,
        .minimum = minimum,
        .maximum = maximum,
        .widthIntrinsicSizes = widthIntrinsicSizes,
        .heightIntrinsicSizes = heightIntrinsicSizes,
    };
}
