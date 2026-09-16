//
//  sdf_helpers.cpp
//  gui
//
//  Created by Taanish Reja on 8/24/25.
//

#include "sdf_helpers.hpp"
#include <simd/vector_types.h>

float rounded_rect_sdf(simd_float2 pt, simd_float2 halfExtent, const style::CornerRadii& radii) {
    const float epsilon = 0.0001;

    simd_float2 top = pt.x >= 0.0f ? radii.topRight : radii.topLeft;
    simd_float2 bottom = pt.x >= 0.0f ? radii.bottomRight : radii.bottomLeft;
    simd_float2 r = pt.y >= 0.0f ? bottom : top;

    r.x = std::max(r.x, epsilon);
    r.y = std::max(r.y, epsilon);

    simd_float2 q = simd::abs(pt) - halfExtent + r;
    
    simd_float2 qNormalized = simd_float2{
        std::max(q.x, 0.0f),
        std::max(q.y, 0.0f)
    } / r;
    

    float distOutside = (simd::length(qNormalized) - 1.0) *  std::min(r.x, r.y);
    float distInside = std::min(std::max(q.x,q.y), 0.0f);

    return distOutside + distInside;
}
