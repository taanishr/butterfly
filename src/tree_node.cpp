#include "tree_node.hpp"

namespace tree {
    uint64_t TreeNode::nextId = 0;

    std::vector<TreeNode*> collectAllNodes(TreeNode* root) {
        std::vector<TreeNode*> nodes;
        std::function<void(TreeNode*)> collect = [&](TreeNode* node) {
            if (!node) return;
            nodes.push_back(node);
            for (auto& child : node->children) {
                collect(child.get());
            }
        };
        collect(root);
        return nodes;
    }

    // paint orders mostly used to skip recursion in hit testing (much faster to do there)
    void assignPaintOrderIndices(TreeNode* node, uint64_t& index) {
        if (!node) {
            return;
        }

        node->paintPreorderIndex = index++;
        for (auto& child : node->children) {
            assignPaintOrderIndices(child.get(), index);
        }
        node->paintPostorderIndex = index++;
    }

    void collectInFlowMembers(TreeNode* node, PaintLayer layer, std::vector<StackingMember>& members, std::vector<TreeNode*>& deferred) {
        for (auto& child : node->children) {
            if (child->shared.position != Position::Static || child->establishesStackingContext()) {
                deferred.push_back(child.get());
                continue;
            }

            members.push_back(StackingMember {
                .node = child.get(),
                .layer = layer,
                .establishesContext = false
            });
            collectInFlowMembers(child.get(), layer, members, deferred);
        }
    }

    void collectStackingMembers(TreeNode* node, PaintLayer layer, std::vector<StackingMember>& members) {
        bool establishesContext = node->establishesStackingContext();
        if (node->shared.position != Position::Static || establishesContext) {
            layer = PaintLayer::Positioned;
        }

        members.push_back(StackingMember {
            .node = node,
            .layer = layer,
            .establishesContext = establishesContext
        });

        if (establishesContext) {
            return;
        }

        // CSS places in non positioned members (static/inflow) in the stack context first
        std::vector<TreeNode*> deferred;
        collectInFlowMembers(node, layer, members, deferred);
        for (auto* child : deferred) {
            collectStackingMembers(child, layer, members);
        }
    }

    void createStackingContexts(TreeNode* node, std::vector<TreeNode*>& order) {
        std::vector<StackingMember> members;
        for (auto& child : node->children) {
            collectStackingMembers(child.get(), PaintLayer::InFlow, members);
        }

        std::stable_sort(members.begin(), members.end(), [](const StackingMember& a, const StackingMember& b) {
            auto aZIndex = a.node->zIndex.value_or(0);
            auto bZIndex = b.node->zIndex.value_or(0);
            if (aZIndex != bZIndex) {
                return aZIndex < bZIndex;
            }
            return a.layer < b.layer;
        });

        order.push_back(node);
        for (auto& member : members) {
            if (member.establishesContext) {
                createStackingContexts(member.node, order);
            } else {
                order.push_back(member.node);
            }
        }
    }

}
