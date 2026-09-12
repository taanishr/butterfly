#pragma once

#include <span>
#include <string_view>

namespace tree {
    struct TreeNode;
}

namespace layout_test::scenes {
    tree::TreeNode* build(std::string_view name);
    void buildBrowser();
    std::span<const std::string_view> names();
}
