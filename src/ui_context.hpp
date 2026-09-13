#pragma once

#include "buffer_allocator.hpp"
#include "frame_info.hpp"
#include "layout/layout.hpp"
#include "metal_imports.hpp"
#include <atomic>
#include <cstdint>

namespace runtime {
    struct UIContext {
        UIContext(MTL::Device* device, MTK::View* view);

        void updateView();

        MTL::Device* device;
        MTK::View* view;
        DrawableBufferAllocator allocator;
        layout::LayoutEngine layoutEngine;
        FrameInfo frameInfo;
        DrawableBuffer frameInfoBuffer;
        std::atomic<uint64_t> frameIndex{0};
    };
}
