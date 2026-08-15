#include "new_sizing.hpp"
#include "element.hpp"
#include "new_arch.hpp"
#include "overloaded.hpp"
#include "render_tree.hpp"
#include "sizing.hpp"
#include <MacTypes.h>
#include <optional>
#include <print>
#include <sys/kauth.h>
#include <variant>


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
                case style::Unit::Percent:
                    return std::visit(Overloaded{
                        [&](const style::Size& availableSize) -> SizeState {
                            if (availableSize.unit == style::Unit::Px || availableSize.unit == style::Unit::Pt)
                                return size.value * availableSize.value;

                            return style::SizeError::IndefiniteBasis;
                        },
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
                        }
                    }, available);
            }

            return std::monostate{};
        },
        [](const auto& other) -> SizeState {
            return SizeState{other};
        }
    }, size);
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

    // bool shouldResolvePercentWidth = !constraints.shrinkWidthToFit &&
    //                                 std::holds_alternative<std::monostate>(constraints.parentOverride.width) &&
    //                                 node->shared.width.unit == layout::Unit::Percent;

    // bool shouldResolvePercentHeight = !constraints.shrinkHeightToFit &&
    //                                     std::holds_alternative<std::monostate>(constraints.parentOverride.height) &&
    //                                     node->shared.height.unit == layout::Unit::Percent;

    // if (shouldResolvePercentWidth || shouldResolvePercentWidth) {
    //     layout::SizeResolutionContext sizeCtx {
    //         .position = node->shared.position,
    //         .parentConstraints = constraints,
    //         .top = node->shared.top,
    //         .right = node->shared.right,
    //         .bottom = node->shared.bottom,
    //         .left = node->shared.left,
    //         .requestedWidth = node->shared.width,
    //         .requestedHeight = node->shared.height,
    //         .availableWidth = constraints.availableWidth,
    //         .availableHeight = constraints.availableHeight
    //     };

    //     auto newSize = resolveSize(sizeCtx);

    //     // std::println("new size x: {} new size y: {}, old size x: {}, old size y: {}", newSize.width, newSize.height, size.width, size.height);
    // }

    SizePair minimum {
        .width = calculateSize(req.minimum.width, req.available.width),
        .height = calculateSize(req.minimum.height, req.available.height),
    };
    SizePair maximum {
        .width = calculateSize(req.maximum.width, req.available.width),
        .height = calculateSize(req.maximum.height, req.available.height),
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
