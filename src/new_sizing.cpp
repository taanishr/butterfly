#include "new_sizing.hpp"
#include "element.hpp"
#include "hash_combine.hpp"
#include "new_arch.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <algorithm>
#include <cstddef>
#include <optional>
#include <variant>

// hash helpers
auto hashSize(const SizeState& size, std::size_t& key) -> void {
    hash_combine(key, size.index());
    std::visit(Overloaded {
        [](std::monostate) {},
        [&](const style::Size& size) { hash_combine(key, size.value); hash_combine(key, size.unit); },
        [&](float value) { hash_combine(key, value); },
        [&](SizeError error) { hash_combine(key, error); },
    }, size);
};

auto hashSizePair(const SizePair& size, std::size_t& key) -> void {
    hashSize(size.width, key);
    hashSize(size.height, key);
}

auto hashSizeRequest(const SizeRequest& sizeRequest, std::size_t& key) -> void {
    hash_combine(key, sizeRequest.position);

    hashSizePair(sizeRequest.specified, key);
    hashSizePair(sizeRequest.override, key);
    hashSizePair(sizeRequest.content, key);
    hashSizePair(sizeRequest.minimum, key);
    hashSizePair(sizeRequest.maximum, key);
    hashSizePair(sizeRequest.available, key);

    hash_combine(key, sizeRequest.top.has_value());
    if (sizeRequest.top.has_value()) {
        hashSize(*sizeRequest.top, key);
    }

    hash_combine(key, sizeRequest.right.has_value());
    if (sizeRequest.right.has_value()) {
        hashSize(*sizeRequest.right, key);
    }

    hash_combine(key, sizeRequest.bottom.has_value());
    if (sizeRequest.bottom.has_value()) {
        hashSize(*sizeRequest.bottom, key);
    }

    hash_combine(key, sizeRequest.left.has_value());
    if (sizeRequest.left.has_value()) {
        hashSize(*sizeRequest.left, key);
    }

    hashSize(sizeRequest.paddingTop, key);
    hashSize(sizeRequest.paddingRight, key);
    hashSize(sizeRequest.paddingBottom, key);
    hashSize(sizeRequest.paddingLeft, key);
    hashSize(sizeRequest.borderWidth, key);

    hash_combine(key, sizeRequest.margins.top);
    hash_combine(key, sizeRequest.margins.right);
    hash_combine(key, sizeRequest.margins.bottom);
    hash_combine(key, sizeRequest.margins.left);

    hash_combine(key, sizeRequest.aspectRatio.has_value());
    if (sizeRequest.aspectRatio.has_value()) {
        hash_combine(key, *sizeRequest.aspectRatio);
    }

    hash_combine(key, sizeRequest.automaticWidth);
    hash_combine(key, sizeRequest.automaticHeight);
    hash_combine(key, sizeRequest.automaticMinimumWidth);
    hash_combine(key, sizeRequest.automaticMinimumHeight);

    hash_combine(key, sizeRequest.intrinsicWidthRequest.has_value());
    if (sizeRequest.intrinsicWidthRequest.has_value()) {
        hash_combine(key, *sizeRequest.intrinsicWidthRequest);
    }
    hash_combine(key, sizeRequest.intrinsicHeightRequest.has_value());
    if (sizeRequest.intrinsicHeightRequest.has_value()) {
        hash_combine(key, *sizeRequest.intrinsicHeightRequest);
    }

    hash_combine(key, sizeRequest.resolvingIntrinsicWidth);
    hash_combine(key, sizeRequest.resolvingIntrinsicHeight);
}

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
//   - automatic using outer size
// resolve height
//   - specified or intrinsic
//   - automatic from opposing insets
//   - automatic using available size
//   - automatic using outer size
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

        // this is all inner box nonsense....
        // SizeState paddingLeft = calculateSize(req.paddingLeft, req.available.width);
        // const auto* resolvedPaddingLeft = std::get_if<float>(&paddingLeft);

        // if (!resolvedPaddingLeft) {
        //     return paddingLeft;
        // }

        // SizeState paddingRight = calculateSize(req.paddingRight, req.available.width);
        // const auto* resolvedPaddingRight = std::get_if<float>(&paddingRight);

        // if (!resolvedPaddingRight) {
        //     return paddingRight;
        // }

        // SizeState borderWidth = calculateSize(req.borderWidth, req.available.width);
        // const auto* resolvedBorderWidth = std::get_if<float>(&borderWidth);

        // if (!resolvedBorderWidth) {
        //     return borderWidth;
        // }

        float automaticWidth = *availableWidth
            - req.margins.left
            - req.margins.right;
            // - *resolvedPaddingLeft
            // - *resolvedPaddingRight
            // - 2.0f * *resolvedBorderWidth;

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

auto resolveInnerWidth(const SizeState& size, SizeRequest& req) -> SizeState {
    // first; calculate modifiers
    SizeState paddingLeft = calculateSize(req.paddingLeft, req.available.width);
    const auto* resolvedPaddingLeft = std::get_if<float>(&paddingLeft);

    SizeState paddingRight = calculateSize(req.paddingRight, req.available.width);
    const auto* resolvedPaddingRight = std::get_if<float>(&paddingRight);

    SizeState borderWidth = calculateSize(req.borderWidth, req.available.width);
    const auto* resolvedBorderWidth = std::get_if<float>(&borderWidth);

    // copy inner size & modify
    return std::visit(Overloaded{
        [&](float innerSize) -> SizeState {        
            if (resolvedPaddingLeft) {
                innerSize -= *resolvedPaddingLeft;
            }

            if (resolvedPaddingRight) {
                innerSize -= *resolvedPaddingRight;
            }
    
            if (resolvedBorderWidth) {
                innerSize -= 2 * *resolvedBorderWidth;
            }

            return innerSize;
        },
        [&](auto& other) -> SizeState {
            return other;
        }
    }, size);
}

auto resolveHeight(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState {
    // run size through a calculate size pass (maybe avail too)
    SizeState resolved = calculateSize(size, req.available.height);

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

        return resolveIntrinsicHeight(size, *intrinsic, req);
    }

    // if our resulting size is automatic
    if (*error == SizeError::Auto) {
        // are we content sizing?
        if (req.automaticHeight == AutomaticSizing::UseContent) {
            return calculateSize(req.content.height, req.available.height);
        }

        // are we avail sizing?
        // first, refine the available size
        SizeState available = calculateSize(req.available.height, std::monostate{});
        const auto* availableHeight = std::get_if<float>(&available);

        if (!availableHeight) {
            return available;
        }

        // // req.padding* against width is not a bug; that is geniuenly just the spec
        // SizeState paddingTop = calculateSize(req.paddingTop, req.available.width);
        // const auto* resolvedPaddingTop = std::get_if<float>(&paddingTop);

        // if (!resolvedPaddingTop) {
        //     return paddingTop;
        // }

        // SizeState paddingBottom = calculateSize(req.paddingBottom, req.available.width);
        // const auto* resolvedPaddingBottom = std::get_if<float>(&paddingBottom);

        // if (!resolvedPaddingBottom) {
        //     return paddingBottom;
        // }

        // SizeState borderWidth = calculateSize(req.borderWidth, req.available.width);
        // const auto* resolvedBorderWidth = std::get_if<float>(&borderWidth);

        // if (!resolvedBorderWidth) {
        //     return borderWidth;
        // }

        float automaticHeight = *availableHeight
            - req.margins.top
            - req.margins.bottom;
            // - *resolvedPaddingTop
            // - *resolvedPaddingBottom
            // - 2.0f * *resolvedBorderWidth;

        // then, branch from here
        // are we *out of flow* and do we have insets?
        bool outOfFlow = req.position == style::Position::Absolute || req.position == style::Position::Fixed;
        bool opposingInsets = req.top.has_value() && req.bottom.has_value();

        if (outOfFlow && opposingInsets) {
            SizeState top = calculateSize(*req.top, req.available.height);
            const auto* resolvedTop = std::get_if<float>(&top);

            if (!resolvedTop) {
                return top;
            }

            SizeState bottom = calculateSize(*req.bottom, req.available.height);
            const auto* resolvedBottom = std::get_if<float>(&bottom);

            if (!resolvedBottom) {
                return bottom;
            }

            return automaticHeight - *resolvedTop - *resolvedBottom;
        }

        // otherwise: just stretch
        return automaticHeight;
    }

    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
    return resolved;
}

auto resolveInnerHeight(const SizeState& size, SizeRequest& req) {
    // first; calculate modifiers
    // req.padding* against width is not a bug; that is geniuenly just the spec
    SizeState paddingTop = calculateSize(req.paddingTop, req.available.width);
    const auto* resolvedPaddingTop = std::get_if<float>(&paddingTop);

    SizeState paddingBottom = calculateSize(req.paddingBottom, req.available.width);
    const auto* resolvedPaddingBottom = std::get_if<float>(&paddingBottom);

    SizeState borderWidth = calculateSize(req.borderWidth, req.available.width);
    const auto* resolvedBorderWidth = std::get_if<float>(&borderWidth);

    // copy inner size & modify
    return std::visit(Overloaded{
        [&](float innerSize) -> SizeState {        
            if (resolvedPaddingTop) {
                innerSize -= *resolvedPaddingTop;
            }

            if (resolvedPaddingBottom) {
                innerSize -= *resolvedPaddingBottom;
            }
    
            if (resolvedBorderWidth) {
                innerSize -= 2 * *resolvedBorderWidth;
            }

            return innerSize;
        },
        [&](auto& other) -> SizeState {
            return other;
        }
    }, size);
}

// these ONLY exist because of different auto behavior fo min/max width and height
auto resolveMinWidth(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState {
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

    // if our resulting size is automatic (differs from above):
    if (*error == SizeError::Auto) {
        // usually, floor it to 0 (the automatic minimum)
        if (req.automaticMinimumWidth == AutomaticMinimum::Zero) {
            return 0.0f;
        }

        if (!intrinsic) {
            return SizeError::ContentDependent;
        }

        return intrinsic->minimum;
    }

    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
    return resolved;
}

auto resolveMaxWidth(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState {
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

    // there is no automatic branch; i do not want to throw an error.
    //  maybe no-op this? Dont do anything?
    if (*error == SizeError::Auto) {
        return std::monostate{};
    }
    
    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
    return resolved;
}

auto resolveMinHeight(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState {
    // run size through a calculate size pass (maybe avail too)
    SizeState resolved = calculateSize(size, req.available.height);

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

        return resolveIntrinsicHeight(size, *intrinsic, req);
    }

    // if our resulting size is automatic (differs from above):
    if (*error == SizeError::Auto) {
        // usually, floor it to 0 (the automatic minimum)
        if (req.automaticMinimumHeight == AutomaticMinimum::Zero) {
            return 0.0f;
        }

        if (!intrinsic) {
            return SizeError::ContentDependent;
        }

        return intrinsic->minimum;
    }

    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
    return resolved;
}

auto resolveMaxHeight(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState  {
    // run size through a calculate size pass (maybe avail too)
    SizeState resolved = calculateSize(size, req.available.height);

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

        return resolveIntrinsicHeight(size, *intrinsic, req);
    }

    // there is no automatic branch; i do not want to throw an error.
    //  maybe no-op this? Dont do anything?
    if (*error == SizeError::Auto) {
        return std::monostate{};
    }
    
    // should we just early return, or mutate a starting size?
    // not clear to me that mutation is necessarily good
    return resolved;
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

auto measureIntrinsicWidth(
    tree::RenderTree& tree,
    tree::TreeNode* node,
    const FrameInfo& frameInfo,
    layout::Constraints constraints,
    layout::Measured measured,
    const SizeState& antiSize,
    SizeRequest req
) -> IntrinsicResult {
    // antiSize: not used here

    if (req.resolvingIntrinsicWidth) {
        return {
            .minimum = SizeError::ContentDependent,
            .maximum = SizeError::ContentDependent,
        };
    }

    // first; create a new request (or have the recursive tree func do this)
    // this new request should set resolvingIntrinsicWidth = true
    req.resolvingIntrinsicWidth = true;
    // legacy override
    measured.explicitWidth = std::unexpected(style::SizeError::Auto);

    // content.width may have been populated by an earlier resize pass from the
    // node's used layout width. Carrying that cached measurement into this new
    // intrinsic pass makes an automatic width resolve back to the previous used
    // width, which then gets reported and collected as the intrinsic width. Start
    // without that prior output so this pass measures the node's actual content.
    req.override.width = style::Size::autoSize();
    req.content.width = std::monostate{};
    req.automaticWidth = AutomaticSizing::UseContent;
        

    // afterwards, establish the recursive call
    IntrinsicResult result;
    /*
        BIG TODO: (not yet done, and kind of only semi-related to this function)
        create an abstraction to make the inline text breaking algorithm less dependent
        on the leakiness of size; maybe we should create a separate
        InlineContext? that the buildInlineBoxes take in
        and that is generated by a function here (ok this was kind of done alr)
    */


    if (req.intrinsicWidthRequest == IntrinsicRequest::Both) {
        SizeRequest minReq = req;
        minReq.intrinsicWidthRequest = IntrinsicRequest::Minimum;
        // perhaps make a distinc mode vs request type? but that would be hella weird to acc wire
        minReq.intrinsicWidthRequest = IntrinsicRequest::Minimum;
        std::optional<layout::IntrinsicSizes> minIntrinsic = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, minReq);

        SizeRequest maxReq = req;

        maxReq.intrinsicWidthRequest = IntrinsicRequest::Maximum;
        std::optional<layout::IntrinsicSizes> maxIntrinsic = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, maxReq);

        // if (node->id == 104) {
        //     std::println("avail: {} specified: {} min intrinsic: {}, max intrinsic: {}", describeSizeState(req.available.width), describeSizeState(req.specified.width), describeSizeState(minIntrinsic->minimum), describeSizeState(maxIntrinsic->maximum));
        // }

        result = {
            .minimum = minIntrinsic->minimum,
            .maximum = maxIntrinsic->maximum
        };
    }else {
        auto intrinsic = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, req);

        if (!intrinsic) {
            return {
                .minimum = SizeError::ContentDependent,
                .maximum = SizeError::ContentDependent,
            };
        }

        result = {
            .minimum = intrinsic->minimum,
            .maximum = intrinsic->maximum
        };
    }

    return result;
}

auto measureIntrinsicHeight(
    tree::RenderTree& tree,
    tree::TreeNode* node,
    const FrameInfo& frameInfo,
    layout::Constraints constraints,
    layout::Measured measured,
    const SizeState& antiSize,
    SizeRequest req
) -> IntrinsicResult {
    // antisize: USED HERE

    if (req.resolvingIntrinsicHeight) {
        return {
            .minimum = SizeError::ContentDependent,
            .maximum = SizeError::ContentDependent,
        };
    }

    // first; create a new request (or have the recursive tree func do this)
    // this new request should set resolvingIntrinsicHeight = true
    req.resolvingIntrinsicHeight = true;
    
    measured.explicitHeight = std::unexpected(style::SizeError::Auto);

    req.override.height = style::Size::autoSize();
    req.content.height = std::monostate{};
    req.automaticHeight = AutomaticSizing::UseContent;

    // if an antiSize has been resolved and provided via antisize, the request SHOULD note this
    SizeState resolvedAntiSize = calculateSize(antiSize, req.available.width);
    const auto* resolvedWidth = std::get_if<float>(&resolvedAntiSize);

    if (resolvedWidth) {
        req.override.width = *resolvedWidth;
    }

    IntrinsicResult result;

    // establish the recursive call; make sure it establishes the specified antiSize correcttly
    if (req.intrinsicHeightRequest == IntrinsicRequest::Both) {
        SizeRequest minReq = req;
        // perhaps make a distinc mode vs request type? but that would be hella weird to acc wire
        minReq.intrinsicHeightRequest = IntrinsicRequest::Minimum;
        std::optional<layout::IntrinsicSizes> minIntrinsic = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, minReq);

        SizeRequest maxReq = req;
        maxReq.intrinsicHeightRequest = IntrinsicRequest::Maximum;
        std::optional<layout::IntrinsicSizes> maxIntrinsic = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, maxReq);

        // set both the min and max intrinsic size
        result = {
            .minimum = minIntrinsic->minimum,
            .maximum = maxIntrinsic->maximum
        };
    }else {
        auto intrinsic = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, req);

        if (!intrinsic) {
            return {
                .minimum = SizeError::ContentDependent,
                .maximum = SizeError::ContentDependent,
            };
        }

        // set both the min and max intrinsic size
        result = {
            .minimum = intrinsic->minimum,
            .maximum = intrinsic->maximum
        };
    }

    return result;
}

// determine based on min/max/fit content and provided sizes
auto resolveIntrinsicWidth(const SizeState& size, const IntrinsicResult& intrinsic, SizeRequest& req) -> SizeState {
    // fairly simple
    // take the min and max, and according to the requested size:
        // request requires min
            // return min
        // request requires max
            // return max
        // request requires fit content
            // clampSize with min and max set

    /*
        Non-intrinsic values fall back to ordinary size calculation.
    */

    const auto* request = std::get_if<style::Size>(&size);

    if (!request) {
        return size;
    }

    switch (request->unit) {
        case style::Unit::MinContent:
            return intrinsic.minimum;
        case style::Unit::MaxContent:
            return intrinsic.maximum;
        case style::Unit::FitContent: {
            SizeState available = calculateSize(req.available.width, std::monostate{});
            return clampSize(available, intrinsic.minimum, intrinsic.maximum);
        }
        default:
            return calculateSize(size, req.available.width);
    }
}
auto resolveIntrinsicHeight(const SizeState& size, const IntrinsicResult& intrinsic, SizeRequest& req) -> SizeState {
    // fairly simple
    // take the min and max, and according to the requested size:
        // request requires min
            // return min
        // request requires max
            // return max
        // request requires fit content
            // clampSize with min and max set

    /*
        Non-intrinsic values fall back to ordinary size calculation.
    */

    const auto* request = std::get_if<style::Size>(&size);

    if (!request) {
        return size;
    }

    switch (request->unit) {
        case style::Unit::MinContent:
            return intrinsic.minimum;
        case style::Unit::MaxContent:
            return intrinsic.maximum;
        case style::Unit::FitContent: {
            SizeState available = calculateSize(req.available.height, std::monostate{});
            return clampSize(available, intrinsic.minimum, intrinsic.maximum);
        }
        default:
            return calculateSize(size, req.available.height);
    }
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
    SizeRequest req,
    std::optional<std::unordered_map<size_t, SizeResult>&> sizeCache
) -> SizeResult
{
    /*
        flow of func:
        
        resolve preferred width and height
        resolve width/height minimums and maximums

        if width was requested as auto, remains unresolved, and height is numeric:
            transfer height into width

        if preferred/minimum/maximum width needs intrinsic facts:
            measure intrinsic width
            rerun only the width operations that needed those facts

        clamp width

        if height was requested as auto, remains unresolved, and width is numeric:
            transfer width into height

        if preferred/minimum/maximum height needs intrinsic facts:
            measure intrinsic height using the clamped width
            rerun only the height operations that needed those facts

        clamp height
        return the partial or complete result  
    */

    size_t sizeKey = 0;
    hash_combine(sizeKey, node);
    hashSizeRequest(req, sizeKey);

    const bool traceBodyHeight = node->id == 21 && req.intrinsicHeightRequest.has_value();
    if (traceBodyHeight) {
        SizeState measuredHeight = measured.explicitHeight
            ? SizeState{*measured.explicitHeight}
            : SizeState{measured.explicitHeight.error()};
        std::println(
            "[Body height sizing input] resolving={} available-height={} constraint-height={} measured-height={} specified-height={} override-height={} content-height={}",
            req.resolvingIntrinsicHeight,
            describeSizeState(req.available.height),
            describeSize(constraints.availableHeight),
            describeSizeState(measuredHeight),
            describeSizeState(req.specified.height),
            describeSizeState(req.override.height),
            describeSizeState(req.content.height)
        );
    }

    // const bool traceNode100Intrinsic = node->id == 100 && req.intrinsicWidthRequest.has_value();
    // if (traceNode100Intrinsic) {
    //     SizeState measuredWidth = measured.explicitWidth
    //         ? SizeState{*measured.explicitWidth}
    //         : SizeState{measured.explicitWidth.error()};
    //     std::println(
    //         "[child 100 intrinsic input] resolving={} request-available-width={} constraint-available-width={} measured-width={} override-width={} content-width={}",
    //         req.resolvingIntrinsicWidth,
    //         describeSizeState(req.available.width),
    //         describeSize(constraints.availableWidth),
    //         describeSizeState(measuredWidth),
    //         describeSizeState(req.override.width),
    //         describeSizeState(req.content.width)
    //     );
    // }

    if (sizeCache) {
        auto it = sizeCache->find(sizeKey);
        if (it != sizeCache->end()) {
            if (traceBodyHeight) {
                std::println(
                    "[Body height sizing cache] hit outer-height={} minimum-height={}",
                    describeSizeState(it->second.outerSize.height),
                    describeSizeState(it->second.minimum.height)
                );
                if (it->second.heightIntrinsicSizes) {
                    std::println(
                        "[Body height sizing cache] intrinsic-min-height={} intrinsic-max-height={}",
                        describeSizeState(it->second.heightIntrinsicSizes->minimum),
                        describeSizeState(it->second.heightIntrinsicSizes->maximum)
                    );
                }
            }
            // if (traceNode100Intrinsic) {
            //     std::println(
            //         "[child 100 intrinsic cache] hit outer-width={}",
            //         describeSizeState(it->second.outerSize.width)
            //     );
            //     if (it->second.widthIntrinsicSizes) {
            //         std::println(
            //             "[child 100 intrinsic cache] returned-min-width={} returned-max-width={}",
            //             describeSizeState(it->second.widthIntrinsicSizes->minimum),
            //             describeSizeState(it->second.widthIntrinsicSizes->maximum)
            //         );
            //     } else {
            //         std::println("[child 100 intrinsic cache] returned intrinsic width: none");
            //     }
            // }
            return it->second;
        }
    }

    if (traceBodyHeight) {
        std::println("[Body height sizing cache] miss");
    }

    // if (traceNode100Intrinsic) {
    //     std::println("[child 100 intrinsic cache] miss");
    // }

    const auto& requestedWidth = std::holds_alternative<std::monostate>(req.override.width)
        ? req.specified.width
        : req.override.width;
    const auto& requestedHeight = std::holds_alternative<std::monostate>(req.override.height)
        ? req.specified.height
        : req.override.height;

    const auto* requestedWidthSize = std::get_if<style::Size>(&requestedWidth);
    const auto* requestedHeightSize = std::get_if<style::Size>(&requestedHeight);
    const auto* requestedWidthError = std::get_if<SizeError>(&requestedWidth);
    const auto* requestedHeightError = std::get_if<SizeError>(&requestedHeight);

    bool automaticWidth = (requestedWidthSize && requestedWidthSize->isAuto()) || (requestedWidthError && *requestedWidthError == SizeError::Auto);
    bool automaticHeight = (requestedHeightSize && requestedHeightSize->isAuto()) || (requestedHeightError && *requestedHeightError == SizeError::Auto);

    SizePair size {
        .width = resolveWidth(requestedWidth, req, std::nullopt),
        .height = resolveHeight(requestedHeight, req, std::nullopt),
    };

    SizePair minimum {
        .width = resolveMinWidth(req.minimum.width, req, std::nullopt),
        .height = resolveMinHeight(req.minimum.height, req, std::nullopt),
    };

    SizePair maximum {
        .width = resolveMaxWidth(req.maximum.width, req, std::nullopt),
        .height = resolveMaxHeight(req.maximum.height, req, std::nullopt),
    };

    const auto* widthError = std::get_if<SizeError>(&size.width);
    const auto* minWidthError = std::get_if<SizeError>(&minimum.width);
    const auto* maxWidthError = std::get_if<SizeError>(&maximum.width);
    const auto* heightError = std::get_if<SizeError>(&size.height);
    const auto* minHeightError = std::get_if<SizeError>(&minimum.height);
    const auto* maxHeightError = std::get_if<SizeError>(&maximum.height);

    bool widthIntrinsicError = widthError && *widthError == SizeError::ContentDependent;
    bool minWidthIntrinsicError = (minWidthError && *minWidthError == SizeError::ContentDependent)
                                || req.intrinsicWidthRequest == IntrinsicRequest::Minimum
                                || req.intrinsicWidthRequest == IntrinsicRequest::Both;
    bool maxWidthIntrinsicError = (maxWidthError && *maxWidthError == SizeError::ContentDependent)
                                || req.intrinsicWidthRequest == IntrinsicRequest::Maximum
                                || req.intrinsicWidthRequest == IntrinsicRequest::Both;
                                
    bool heightIntrinsicError = heightError && *heightError == SizeError::ContentDependent;
    bool minHeightIntrinsicError = (minHeightError && *minHeightError == SizeError::ContentDependent)
                                    || req.intrinsicHeightRequest == IntrinsicRequest::Minimum
                                    || req.intrinsicHeightRequest == IntrinsicRequest::Both;
    bool maxHeightIntrinsicError = (maxHeightError && *maxHeightError == SizeError::ContentDependent)
                                    || req.intrinsicHeightRequest == IntrinsicRequest::Maximum
                                    || req.intrinsicHeightRequest == IntrinsicRequest::Both;

    if (req.aspectRatio && automaticWidth && !std::holds_alternative<float>(size.width) && std::holds_alternative<float>(size.height)) {
        SizePair transferred = transferAspectRatio(size, *req.aspectRatio);
        size.width = transferred.width;
    }

    std::optional<IntrinsicResult> widthIntrinsic;

    // how do I measure both sizing modes?
    if (widthIntrinsicError || minWidthIntrinsicError || maxWidthIntrinsicError) {
        widthIntrinsic = measureIntrinsicWidth(tree, node, frameInfo, constraints, measured, size.height, req);

        if (widthIntrinsicError) {
            size.width = resolveWidth(requestedWidth, req, widthIntrinsic);
        }
        if (minWidthIntrinsicError) {
            minimum.width = resolveMinWidth(req.minimum.width, req, widthIntrinsic);
        }
        if (maxWidthIntrinsicError) {
            maximum.width = resolveMaxWidth(req.maximum.width, req, widthIntrinsic);
        }
    }

    size.width = clampSize(size.width, minimum.width, maximum.width);

    if (req.aspectRatio && automaticHeight && !std::holds_alternative<float>(size.height) && std::holds_alternative<float>(size.width)) {
        SizePair transferred = transferAspectRatio(size, *req.aspectRatio);
        size.height = transferred.height;
    }

    std::optional<IntrinsicResult> heightIntrinsic;

    if (heightIntrinsicError || minHeightIntrinsicError || maxHeightIntrinsicError) {
        heightIntrinsic = measureIntrinsicHeight(tree, node, frameInfo, constraints, measured, size.width, req);

        if (heightIntrinsicError) {
            size.height = resolveHeight(requestedHeight, req, heightIntrinsic);
        }
        if (minHeightIntrinsicError) {
            minimum.height = resolveMinHeight(req.minimum.height, req, heightIntrinsic);
        }
        if (maxHeightIntrinsicError) {
            maximum.height = resolveMaxHeight(req.maximum.height, req, heightIntrinsic);
        }
    }

    SizeState preferredHeightBeforeClamp = size.height;
    size.height = clampSize(size.height, minimum.height, maximum.height);

    if (traceBodyHeight) {
        std::println(
            "[Body height sizing result] preferred-before-clamp={} minimum-height={} final-outer-height={}",
            describeSizeState(preferredHeightBeforeClamp),
            describeSizeState(minimum.height),
            describeSizeState(size.height)
        );
        if (heightIntrinsic) {
            std::println(
                "[Body height sizing result] intrinsic-min-height={} intrinsic-max-height={}",
                describeSizeState(heightIntrinsic->minimum),
                describeSizeState(heightIntrinsic->maximum)
            );
        }
    }

    // inner sizes 
    SizePair innerSize {
        .width = resolveInnerWidth(size.width, req),
        .height = resolveInnerHeight(size.height, req)
    };

    SizeResult result = {
        .outerSize = size,
        .innerSize = innerSize,
        .minimum = minimum,
        .maximum = maximum,
        .widthIntrinsicSizes = widthIntrinsic,
        .heightIntrinsicSizes = heightIntrinsic,
    };

    // if (traceNode100Intrinsic) {
    //     std::println(
    //         "[child 100 intrinsic result] outer-width={} inner-width={}",
    //         describeSizeState(result.outerSize.width),
    //         describeSizeState(result.innerSize.width)
    //     );
    //     if (result.widthIntrinsicSizes) {
    //         std::println(
    //             "[child 100 intrinsic result] returned-min-width={} returned-max-width={}",
    //             describeSizeState(result.widthIntrinsicSizes->minimum),
    //             describeSizeState(result.widthIntrinsicSizes->maximum)
    //         );
    //     } else {
    //         std::println("[child 100 intrinsic result] returned intrinsic width: none");
    //     }
    // }

    if (sizeCache) {
        sizeCache->insert({sizeKey, result});
    }

    return result;
}
