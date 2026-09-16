#pragma once

#include "layout/layout.hpp"
#include "ui_context.hpp"
#include <any>
#include <concepts>
#include <string>
#include <variant>

namespace elements {
    template<typename D>
    concept DescriptorType = requires(
        D d,
        std::any payload
    )
    {
        { d.request(payload) } -> std::same_as<std::any>;
    };

    struct GetFull {};
    struct GetField { std::string name; };
    using DescriptorPayload = std::variant<GetFull, GetField>;

    using layout::FragmentID;
    using runtime::UIContext;

    template <typename S>
    struct Fragment {
        Fragment(UIContext& ctx):
            id{Fragment::nextId++},
            fragmentStorage{ctx}
        {}
        
        static FragmentID nextId;
        FragmentID id;
        S fragmentStorage;
    };

    template <typename S>
    FragmentID Fragment<S>::nextId = 0;
}
