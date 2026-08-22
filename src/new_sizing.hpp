#pragma once

// #include "element.hpp"
#include "margins.hpp"
#include "sizing.hpp"
#include <optional>

struct FrameInfo;

namespace layout {
    struct Constraints;
    struct IntrinsicSizes;
    struct Measured;
}

namespace tree {
    struct RenderTree;
    struct TreeNode;
}

// existing problem with current Size
// the current Size is a good descriptor of what the Size is
// but internally, everything should be rerpesented in pixel space
// resolved size is a bad name; ill consider renaming it smth else
using style::SizeError;

// requesters pass context as a SizeSpec request + constraints to evaulator
/*
    things a req should include:
    1) do you want intrinsic SizeSpecs back? (yes/no)
    2) what is the content width?
    3) what is the specified SizeSpec you want? 
    4) is there a parent override?
*/

enum class IntrinsicRequest {
    Minimum,
    Maximum
};


// CHILDREN SHOULD MAKE THEIR OWN SIZING REQUEST based ON parent constraints
// so parents do not provide the request, they provide the information to make a request
enum class AutomaticSizing {
    UseAvailable,
    UseContent,
};

struct SizeRequest {
    SizePair specified; // specified h/w
    SizePair override;  // parent override
    SizePair content;   // content box
    SizePair minimum; // minimums
    SizePair maximum; // maximums
    SizePair available;
    
    ResolvedMargins margins; // margins (req for some sizing)
    
    // these two fields are unnecessary
    // std::optional<IntrinsicRequest> intrinsicWidthRequest {};
    // std::optional<IntrinsicRequest> intrinsicHeightRequest {};

    AutomaticSizing automaticWidth;
    AutomaticSizing automaticHeight;

    bool resolvingIntrinsicWidth{false};
    bool resolvingIntrinsicHeight{false};
};  

// central evaluator gives back a coherently shaped SizeSpec resolution
// SizeSpec *may or may not* be fully resolved; that is fine
struct SizeResult {
    SizePair size;      // preferred SizeSpec recieved back
    SizePair minimum;   // min dim constraints evaluated against content box
    SizePair maximum; // max dim constraints evaluated against content box
    
    std::optional<layout::IntrinsicSizes> widthIntrinsicSizes; // intrinsicSizeSpecs if driven by width constraints
    std::optional<layout::IntrinsicSizes> heightIntrinsicSizes; // intrinsic SizeSpecs if driven by height constraints
};

struct IntrinsicResult {
    SizeState minimum;
    SizeState maximum;
};

// resolve preferred width and height
// top level

// sizing operations to consider

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

// intrinsic:
// intrinsic width (min, max, fit content)
// intrinsic height (min, max, fit content)
// intrinsic height given set width (previously resolved)
// unknown/skip: intrinsic width based on resolved height: unlikely / nonexistent

// min/max:
// resolve min/max width (based on intrinsic / specified)
// resolve min/max height (based on intrinsic / specified)

// clamp min/max
// clamp width according to min/max
// clamp height according to min/max

// aspect ratio:
// determine width from resolved height
// determine height from resolved width

// isolating text from this 
// something has to leak; line breaking requires necessary info about the content request
// however, I can *probably* make a new struct to hide this

// guarding against recursion:
// active boolean for width and height intrinsic requests? default to false?


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
auto resolveWidth(const SizeState& size, SizeRequest& req) -> SizeState;
auto resolveHeight(const SizeState& size, SizeRequest& req) -> SizeState;

// these ONLY exist because of different auto behavior fo min/max widht and height
auto resolveMinWidth(const SizeState& size) -> SizeState;
auto resolveMaxWidth(const SizeState& size) -> SizeState;
auto resolveMinHeight(const SizeState& size) -> SizeState;
auto resolveMaxHeight(const SizeState& size) -> SizeState;

// handles intrinsic cases
// intrinsic width (min, max, fit content)
// intrinsic height (min, max, fit content)

// the anti revolves around these two ops
// intrinsic height given set width (previously resolved)
// unknown/skip: intrinsic width based on resolved height: unlikely / nonexistent
// to start, the anti wil be ignored for resolveIntrinsicWidth
// this should likely not return a size state, but probably a min and max size (intrinsic result type)
// IntrinsicResult? (intrinsic sizes as is is not satisfactory and relies on old Size, which is more of a description than a proper intermediate)

auto measureIntrinsicWidth(const SizeState& size, const SizeState& antiSize) -> IntrinsicResult;
auto measureIntrinsicHeight(const SizeState& size, const SizeState& antiSize) -> IntrinsicResult;

// determine based on min/max/fit content and provided sizes
// req may be overkill, I could move to a specific min/max/fit selector instead
auto resolveIntrinsicWidth(const SizeState& min, const SizeState& max, SizeRequest& req) -> SizeState;
auto resolveIntrinsicHeight(const SizeState& min, const SizeState& max, SizeRequest& req) -> SizeState;


// clamp a size
// i went through two design ideas:
// clampSizeAgainstMin & clampSizeAgainstMax separation 
// i decided on a unified version, where you pass monostate if one does not exist
// this will probably end up in a few nested variants
// this may cause issues:
// issue 1: what if min > max? min wins
// issue 2: does this create an implied fit content operation? basically, if neither is monostate
auto clampSize(const SizeState& size, const SizeState& min, const SizeState& max) -> SizeState;

// this will also probably end up as a massive function switching between possibilites
// no height, no width
// height, no width
// width, no height
// height, width
auto transferAspectRatio(const SizePair& pair, float ratio) -> SizePair;

// todos:
// some structs for inline text methdos to consume (invariably, they will need some size bounds + content sizing guidelines) 
// active marker; how do I want to shape this? (done)

// resolve what can be resolved from size + available (this is essentially the lowest level primitive of sizing)
auto calculateSize(const SizeState& size, const SizeState& available) -> SizeState;


auto evaluateSize(
    tree::RenderTree& tree,
    tree::TreeNode* node,
    const FrameInfo& frameInfo,
    layout::Constraints constraints,
    layout::Measured measured,
    SizeRequest req
) -> SizeResult;
