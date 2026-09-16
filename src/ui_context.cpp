//
//  ui_context.cpp
//  gui
//
//  Created by Taanish Reja on 11/20/25.
//

#include "ui_context.hpp"
#include "fragment_types.hpp"
#include "overloaded.hpp"
#include "layout/sizing.hpp"
#include <algorithm>
#include <optional>
#include <simd/vector_types.h>


namespace runtime {

    // pipeline specific
    UIContext::UIContext(MTL::Device* device, MTK::View* view):
        device{device},
        view{view},
        allocator{DrawableBufferAllocator{device}},
        layoutEngine{},
        frameInfoBuffer{allocator.allocate(sizeof(FrameInfo))},
        frameIndex{0}
    {
        auto frameDimensions = this->view->drawableSize();
        auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(view));

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};

        this->frameInfo = frameInfo;
        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    };

    void UIContext::updateView() {
        auto frameDimensions = this->view->drawableSize();

        auto scale = AppKit_Extensions::getContentScaleFactor(reinterpret_cast<void*>(view));

        FrameInfo frameInfo {.width=static_cast<float>(frameDimensions.width)/2.0f, .height=static_cast<float>(frameDimensions.height)/2.0f, .scale = scale};
        this->frameInfo = frameInfo;

        std::memcpy(frameInfoBuffer.get()->contents(), &frameInfo, sizeof(FrameInfo));
    }

}
