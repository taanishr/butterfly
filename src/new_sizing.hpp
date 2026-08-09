#include <optional>

// existing problem with current Size
// the current size is a good descriptor of what the size is
// but internally, everything should be rerpesented in pixel space
// resolved size is a bad name; ill consider renaming it smth else
struct ResolvedSize {
    std::optional<float> width;
    std::optional<float> height;
};

// requesters pass context as a size request + constraints to evaulator
/*
    things a req should include:
    1) do you want intrinsic sizes back? (yes/no)
    2) what is the content width?
    3) what is the specified size you want? 
    4) is there a parent override?
*/

// CHILDREN SHOULD MAKE THEIR OWN SIZING REQUEST based ON parent constraints
// so parents do not provide the request, they provide the information to make a request
struct SizeRequest {
    ResolvedSize specified; // specified h/w
    ResolvedSize override;  // parent override
    ResolvedSize content;   // content box
    
    bool requestWidthIntrinsicSize {};
    bool requestHeightIntrinsicSize {};
};  

// central evaluator gives back a coherently shaped size resolution
// size *may or may not* be fully resolved; that is fine
struct SizeResolution {
    ResolvedSize size;      // preferred size recieved back
    ResolvedSize minimum;   // min width constraint evaluated against content box
    ResolvedSize maximum;   // max width constraint evaluated against content box
    
    ResolvedSize widthIntrinsicSizes; // intrinsicSizes if driven by width constraints
    ResolvedSize heightIntrinsicSizes; // intrinsic sizes if driven by height constraints
};

auto evaluateSize(SizeRequest) -> SizeResolution;