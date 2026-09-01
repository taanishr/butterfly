#pragma once

// #include "element.hpp"
#include "margins.hpp"
#include "overloaded.hpp"
#include "sizing.hpp"
#include <format>
#include <optional>
#include <string>
#include <unordered_map>

inline std::string describeSize(const style::Size& size) {
    switch (size.unit) {
        case style::Unit::Px:         return std::format("{}px", size.value);
        case style::Unit::Percent:    return std::format("{}%", size.value * 100.0f);
        case style::Unit::Auto:       return "auto";
        case style::Unit::Pt:         return std::format("{}pt", size.value);
        case style::Unit::Fr:         return std::format("{}fr", size.value);
        case style::Unit::MinContent: return "min-content";
        case style::Unit::MaxContent: return "max-content";
        case style::Unit::FitContent: return "fit-content";
    }
}

inline std::string describeSizeState(const SizeState& size) {
    return std::visit(Overloaded {
        [](std::monostate) { return std::string{"monostate"}; },
        [](float value) { return std::format("{}px", value); },
        [](const style::Size& value) { return describeSize(value); },
        [](style::SizeError error) {
            switch (error) {
                case style::SizeError::Auto:                    return std::string{"error:auto"};
                case style::SizeError::IndefiniteBasis:         return std::string{"error:indefinite-basis"};
                case style::SizeError::FractionRequiresContext: return std::string{"error:fraction-requires-context"};
                case style::SizeError::ContentDependent:        return std::string{"error:content-dependent"};
            }
        },
    }, size);
}


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

namespace style {
    enum class Position;
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
    Maximum,
    Both
};

struct InlineSizingInput {
    SizeState availableWidth;
    std::optional<IntrinsicRequest> widthRequest;
    bool trackIntrinsicWidth;
};


// CHILDREN SHOULD MAKE THEIR OWN SIZING REQUEST based ON parent constraints
// so parents do not provide the request, they provide the information to make a request
enum class AutomaticSizing {
    UseAvailable,
    UseContent,
};

enum class AutomaticMinimum {
    Zero,
    ContentBased,
};

struct SizeRequest {
    style::Position position;

    SizePair specified; // specified h/w
    SizePair override;  // externally imposed size for this evaluation
    SizePair content;   // content box
    SizePair minimum; // minimums
    SizePair maximum; // maximums
    SizePair available;

    std::optional<SizeState> top;
    std::optional<SizeState> right;
    std::optional<SizeState> bottom;
    std::optional<SizeState> left;

    SizeState paddingTop;
    SizeState paddingRight;
    SizeState paddingBottom;
    SizeState paddingLeft;

    SizeState borderWidth;
    ResolvedMargins margins; // margins (req for some sizing)

    std::optional<float> aspectRatio;
    
    AutomaticSizing automaticWidth;
    AutomaticSizing automaticHeight;
    AutomaticMinimum automaticMinimumWidth;
    AutomaticMinimum automaticMinimumHeight;

    // overrides if a consumer specifically wants them (i.e. flex)
    std::optional<IntrinsicRequest> intrinsicWidthRequest;
    std::optional<IntrinsicRequest> intrinsicHeightRequest;

    bool resolvingIntrinsicWidth{false};
    bool resolvingIntrinsicHeight{false};

    // debug tag
    std::optional<std::string> tag {std::nullopt};
};  

struct IntrinsicResult {
    SizeState minimum;
    SizeState maximum;
};

// central evaluator gives back a coherently shaped SizeSpec resolution
// SizeSpec *may or may not* be fully resolved; that is fine
struct SizeResult {
    SizePair outerSize;      // preferred SizeSpec recieved back
    SizePair innerSize;      // inner size
    SizePair minimum;   // min dim constraints evaluated against content box
    SizePair maximum; // max dim constraints evaluated against content box
    
    std::optional<IntrinsicResult> widthIntrinsicSizes;
    std::optional<IntrinsicResult> heightIntrinsicSizes;
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
auto resolveWidth(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState;
auto resolveHeight(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState;

// these ONLY exist because of different auto behavior fo min/max widht and height
auto resolveMinWidth(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState;
auto resolveMaxWidth(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState;
auto resolveMinHeight(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState;
auto resolveMaxHeight(const SizeState& size, SizeRequest& req, const std::optional<IntrinsicResult>& intrinsic) -> SizeState;

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
) -> IntrinsicResult;
auto measureIntrinsicHeight(
    tree::RenderTree& tree,
    tree::TreeNode* node,
    const FrameInfo& frameInfo,
    layout::Constraints constraints,
    layout::Measured measured,
    const SizeState& antiSize,
    SizeRequest req
) -> IntrinsicResult;

// determine based on min/max/fit content and provided sizes
// req may be overkill, I could move to a specific min/max/fit selector instead
auto resolveIntrinsicWidth(const SizeState& size, const IntrinsicResult& intrinsic, SizeRequest& req) -> SizeState;
auto resolveIntrinsicHeight(const SizeState& size, const IntrinsicResult& intrinsic, SizeRequest& req) -> SizeState;


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
    SizeRequest req,
    std::optional<std::unordered_map<size_t, SizeResult>&> sizeCache = std::nullopt
) -> SizeResult;
