#include "new_sizing.hpp"
#include "element.hpp"
#include "render_tree.hpp"
#include <optional>


// circular problem:
// constraints -> determine request
// but intrinsic SizeSpec measurement -> requires constraints that largely mirror existing ones?
// so define a clear separation.
// thats fine, bc both of them are separate constraints

auto evaluateSizeSpec(SizeRequest req) -> SizeResult 
{
    SizeState size {};
    SizeState minimum {};
    SizeState maximum {};

    // // resolve regular SizeSpecs
    // // overide first: then specified width
    // SizeState.width = req.override.width
    //         .or_else([&](){ return req.specified.width; });

    // SizeState.height = req.override.height
    //         .or_else([&](){ return req.specified.height; });

    // // resolve intrinsic SizeSpecs
    std::optional<SizeState> widthIntrinsicSizeSpecs = req.requestWidthIntrinsicSizeSpec.and_then(
    [&](auto& intrinsicReq) -> std::optional<SizeState>{

        switch (intrinsicReq) {
            case IntrinsicRequest::Maximum:
                // return tree.measureIntrdinsicSizeSpecs(node, tree, Constraints constraints, layout::Measured measured, layout::Axis axis);
            case IntrinsicRequest::Minimum:
                return std::nullopt;
        }   
    });

    std::optional<SizeState> heightIntrinsicSizeSpecs = req.requestHeightIntrinsicSizeSpec.and_then(
    [](auto& intrinsicReq) -> std::optional<SizeState>{
        switch (intrinsicReq) {
            case IntrinsicRequest::Maximum:
                return std::nullopt;
            case IntrinsicRequest::Minimum:
                return std::nullopt;
        }   
    });


    return SizeResult {
        .size = {},
        .minimum = {},
        .maximum = {},
        .widthIntrinsicSizeSpecs = widthIntrinsicSizeSpecs,
        .heightIntrinsicSizeSpecs = heightIntrinsicSizeSpecs,
    };
}