#pragma once

// #include "element.hpp"
#include "margins.hpp"
#include "sizing.hpp"
#include <optional>
#include <variant>

// existing problem with current Size
// the current Size is a good descriptor of what the Size is
// but internally, everything should be rerpesented in pixel space
// resolved size is a bad name; ill consider renaming it smth else
using style::SizeError;

// Either a spec, float, or error
using SizeState = std::variant<style::Size, float, SizeError>;

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
    SizeState specified; // specified h/w
    SizeState override;  // parent override
    SizeState content;   // content box
    
    ResolvedMargins margins; // margins (req for some sizing)
    
    std::optional<IntrinsicRequest> requestWidthIntrinsicSizeSpec {};
    std::optional<IntrinsicRequest> requestHeightIntrinsicSizeSpec {};
};  

// central evaluator gives back a coherently shaped SizeSpec resolution
// SizeSpec *may or may not* be fully resolved; that is fine
struct SizeResult {
    SizeState size;      // preferred SizeSpec recieved back
    SizeState minimum;   // min width constraint evaluated against content box
    SizeState maximum;   // max width constraint evaluated against content box
    
    std::optional<SizeState> widthIntrinsicSizeSpecs; // intrinsicSizeSpecs if driven by width constraints
    std::optional<SizeState> heightIntrinsicSizeSpecs; // intrinsic SizeSpecs if driven by height constraints
};

auto evaluateSizeSpec(SizeRequest) -> SizeResult;