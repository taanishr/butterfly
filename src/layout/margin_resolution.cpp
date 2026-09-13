#include "tree_node.hpp"
#include "fragment_types.hpp"
#include "layout/sizing.hpp"
#include "layout/layout.hpp"
#include "render_tree.hpp"
#include "utf8.hpp"
#include "text/textShaper.hpp"
#include <algorithm>
#include <any>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <simd/vector_types.h>
#include <string>
#include <unordered_map>
#include <utility>


namespace tree {
    using elements::DescriptorPayload;
    using elements::GetField;
    using elements::RequestTarget;
    using elements::isTextWhitespace;
    using layout::Constraints;
    using layout::LayoutEngine;
    using layout::LayoutInput;
    using layout::LineBox;
    using layout::LineFragment;
    using style::Display;
    using style::Position;
    using style::Size;
    using style::WhiteSpace;
    using style::WordBreak;

    void precomputeMargins(RenderTree& tree, TreeNode* node, Constraints& constraints, std::unordered_map<ChainID, CollapsedChain>& collapsedChainMap) {
        constraints.replacedAttributes = {};

        if (node->preLayout->marginMetadata.topChainId.has_value()) {
            auto topChain = collapsedChainMap[node->preLayout->marginMetadata.topChainId.value()];
            if (topChain.depth > 1) {
                if (node == topChain.root) {
                    constraints.replacedAttributes.marginTop = topChain.intent;
                } else {
                    constraints.replacedAttributes.marginTop = Size{};
                }
            }
        }

        if (node->preLayout->marginMetadata.bottomChainId.has_value()) {
            auto bottomChain = collapsedChainMap[node->preLayout->marginMetadata.bottomChainId.value()];
            if (bottomChain.depth > 1) {
                if (node == bottomChain.root) {
                    constraints.replacedAttributes.marginBottom = bottomChain.intent;
                } else {
                    constraints.replacedAttributes.marginBottom = Size{};
                }
            }
        }

        auto position = node->getPosition();
        auto display = node->getDisplay();

        Size marginTop = node->getMarginTop();
        Size marginRight = node->getMarginRight();
        Size marginBottom = node->getMarginBottom();
        Size marginLeft = node->getMarginLeft();

        ResolvedMargins margins{};

        if (position == Position::Static && display == Display::Block) {
            // Block normal flow: use resolveAutoMargins
            // Compute content dimensions from atoms
            float contentWidth = 0;
            float contentHeight = 0;
            for (auto& atom : node->atomized->atoms) {
                contentWidth += atom.width;
                contentHeight = std::max(contentHeight, atom.height);
            }

            SizeState resolvedWidth = calculateSize(node->shared.width, constraints.availableWidth);
            if (std::holds_alternative<float>(resolvedWidth)) {
                contentWidth = std::get<float>(resolvedWidth);
            } else if (std::holds_alternative<float>(constraints.availableWidth)) {
                contentWidth = std::get<float>(constraints.availableWidth);
            }

            LayoutInput li{
                .position = position,
                .display = display,
                .marginTop = marginTop,
                .marginRight = marginRight,
                .marginBottom = marginBottom,
                .marginLeft = marginLeft,
            };

            margins = LayoutEngine::resolveAutoMargins(li, constraints.replacedAttributes, constraints.availableWidth, contentWidth);
        } else {
            margins = {
                .top = marginTop.resolveOr(Size::px(0.0f), 0.0f),
                .right = marginRight.resolveOr(Size::px(0.0f), 0.0f),
                .bottom = marginBottom.resolveOr(Size::px(0.0f), 0.0f),
                .left = marginLeft.resolveOr(Size::px(0.0f), 0.0f),
            };
        }

        node->preLayout->resolvedMargins = margins;

        SizeRequest sizeRequest {
            .position = node->shared.position,
            .specified = {.width = node->shared.width, .height = node->shared.height},
            .minimum = {.width = node->shared.minWidth, .height = node->shared.minHeight},
            .maximum = {
                .width = node->shared.maxWidth ? SizeState{*node->shared.maxWidth} : SizeState{std::monostate{}},
                .height = node->shared.maxHeight ? SizeState{*node->shared.maxHeight} : SizeState{std::monostate{}},
            },
            .available = {.width = constraints.availableWidth, .height = constraints.availableHeight},
            .top = node->shared.top,
            .right = node->shared.right,
            .bottom = node->shared.bottom,
            .left = node->shared.left,
            .paddingTop = node->shared.paddingTop.value_or(node->shared.padding),
            .paddingRight = node->shared.paddingRight.value_or(node->shared.padding),
            .paddingBottom = node->shared.paddingBottom.value_or(node->shared.padding),
            .paddingLeft = node->shared.paddingLeft.value_or(node->shared.padding),
            .borderWidth = node->shared.borderWidth,
            .margins = margins,
            .aspectRatio = node->shared.aspectRatio,
            .automaticWidth = (position == Position::Absolute || position == Position::Fixed) ? AutomaticSizing::UseContent : AutomaticSizing::UseAvailable,
            .automaticHeight = AutomaticSizing::UseContent,
            .automaticMinimumWidth = AutomaticMinimum::Zero,
            .automaticMinimumHeight = AutomaticMinimum::Zero,
            .intrinsicWidthRequest = IntrinsicRequest::None,
            .intrinsicHeightRequest = IntrinsicRequest::None,
        };

        SizeResult sizeResult = evaluateSize(tree, node, constraints.frameInfo, constraints, sizeRequest);

        Constraints childConstraints{};
        childConstraints.availableWidth = sizeResult.innerSize.width;
        childConstraints.availableHeight = sizeResult.innerSize.height;
        childConstraints.frameInfo = constraints.frameInfo;

        for (auto& child : node->children) {
            precomputeMargins(tree, child.get(), childConstraints, collapsedChainMap);
        }
    }

}
