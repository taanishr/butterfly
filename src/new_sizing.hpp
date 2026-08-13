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
struct SizeRequest {
    SizePair specified; // specified h/w
    SizePair override;  // parent override
    SizePair content;   // content box
    SizePair minimum; // minimums
    SizePair maximum; // maximums
    SizePair available;
    
    ResolvedMargins margins; // margins (req for some sizing)
    
    // std::optional<IntrinsicRequest> intrinsicWidthRequest {};
    // std::optional<IntrinsicRequest> intrinsicHeightRequest {};
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

auto evaluateSize(
    tree::RenderTree& tree,
    tree::TreeNode* node,
    const FrameInfo& frameInfo,
    layout::Constraints constraints,
    layout::Measured measured,
    SizeRequest req
) -> SizeResult;
