#include "new_sizing.hpp"
#include "element.hpp"
#include "new_arch.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <MacTypes.h>
#include <optional>
#include <variant>


// circular problem:
// constraints -> determine request
// but intrinsic SizeSpec measurement -> requires constraints that largely mirror existing ones?
// so define a clear separation.
// thats fine, bc both of them are separate constraints

auto evaluateSize(
    tree::RenderTree& tree,
    tree::TreeNode* node,
    const FrameInfo& frameInfo,
    layout::Constraints constraints,
    layout::Measured measured,
    SizeRequest req
) -> SizeResult
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
    

    // resolve intrinsic sizes

    // width intrinsic sizes
    auto minWidthIntrinsicError = std::visit(Overloaded{
        [&](style::Size& size) {
            auto availableWidth = std::get_if<style::Size>(&req.available.width);
            
            if (!availableWidth)
                return false;

            auto resolvedWidth = size.resolve(*availableWidth);
            
            return !resolvedWidth && resolvedWidth.error() == style::SizeError::ContentDependent;
        },
        [&](SizeError& err) {
            return err == style::SizeError::ContentDependent;
        },
        [&](auto&) {
            return false;
        }
    }, req.minimum.width);

    auto maxWidthIntrinsicError = std::visit(Overloaded{
        [&](style::Size& size) {
            auto availableWidth = std::get_if<style::Size>(&req.available.width);

            if (!availableWidth)
                return false;

            auto resolvedWidth = size.resolve(*availableWidth);
            return availableWidth && !resolvedWidth && resolvedWidth.error() == style::SizeError::ContentDependent;
        },
        [&](SizeError& err) {
            return err == style::SizeError::ContentDependent;
        },
        [&](auto&) {
            return false;
        }
    }, req.maximum.width);

    auto widthIntrinsicError = std::visit(Overloaded{
        [&](style::Size& size) {
            auto availableWidth = std::get_if<style::Size>(&req.available.width);

            if (!availableWidth)
                return false;

            auto resolvedWidth = size.resolve(*availableWidth);
            return availableWidth && !resolvedWidth && resolvedWidth.error() == style::SizeError::ContentDependent;
        },
        [&](SizeError& err) {
            return err == style::SizeError::ContentDependent;
        },
        [&](auto&) {
            return false;
        }
    }, req.specified.width);

    std::optional<layout::IntrinsicSizes> widthIntrinsicSizes;
    if (constraints.intrinsicSizesAxis != layout::Axis::Width && (minWidthIntrinsicError || maxWidthIntrinsicError || widthIntrinsicError)) {
        widthIntrinsicSizes = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, layout::Axis::Width);
    }
    
    // height intrinsic sizes
    auto heightIntrinsicError = std::visit(Overloaded{
        [&](style::Size& size) {
            auto availableHeight = std::get_if<style::Size>(&req.available.height);

            if (!availableHeight)
                return false;

            auto resolvedHeight = size.resolve(*availableHeight);
            return availableHeight && !resolvedHeight && resolvedHeight.error() == style::SizeError::ContentDependent;
        },
        [&](SizeError& err) {
            return err == style::SizeError::ContentDependent;
        },
        [&](auto&) {
            return false;
        }
    }, req.specified.height);

    std::optional<layout::IntrinsicSizes> heightIntrinsicSizes;
    if (constraints.intrinsicSizesAxis != layout::Axis::Height && heightIntrinsicError) {
        heightIntrinsicSizes = tree.measureIntrinsicSizes(node, frameInfo, constraints, measured, layout::Axis::Height);
    }


    return SizeResult {
        // .wi = {},
        // .minimum = {},
        // .maximum = {},
        .widthIntrinsicSizes = widthIntrinsicSizes,
        .heightIntrinsicSizes = heightIntrinsicSizes,
    };
}
