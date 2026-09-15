//
//  to_ndc.metal
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#pragma once

#include <metal_stdlib>
using namespace metal;

constant constexpr float BASE_PIXEL_HEIGHT = 256.0;

enum class CurveType : uint {
    Quadratic = 0,
    Cubic = 1,
};

struct FrameInfo {
    float width;
    float height;
    float scale;
};

struct CornerRadii {
    float2 topLeft;
    float2 topRight;
    float2 bottomRight;
    float2 bottomLeft;
};

struct ClipUniform {
    float2 rectCenter;
    float2 halfExtent;
    CornerRadii cornerRadius;
};

inline float2 toNDC(const float2 pt, float width = 512.0f, float height = 512.0f) {
    float ndcX = (pt.x / width) * 2.0f - 1.0f;
    float ndcY = 1.0f - (pt.y / height) * 2.0f;
    
    return {ndcX, ndcY};
}

inline float rounded_rect_sdf(float2 pt, float2 halfExtent, CornerRadii radii) {
    const float epsilon = 0.0001;

    // select quadrant radius
    float2 top = select(radii.topLeft, radii.topRight, pt.x >= 0.0);
    float2 bottom = select(radii.bottomLeft, radii.bottomRight, pt.x >= 0.0);
    float2 r = select(top, bottom, pt.y >= 0.0);

    // floor radius x and y (can't be 0 to avoid div by 0, cant be negative)
    r.x = max(r.x, epsilon);
    r.y = max(r.y, epsilon);

    // get the actual signed distance; we add r because we are computing the inner rect
    float2 q = abs(pt) - halfExtent + r;

    // normalize q in terms of the radius units; dist from inner rectnagle to outer rectangle. flat edges are 1 radius away. corners curve to be 1 radius away
    float2 qNormalized = max(q, 0.0) / r;
    

    // get the lengths, subtract 1.0 bc we want 1.0 radiuses away to be the 0 point. less than will be negative (in rect)
    // more than will be positive (outside rect)
    // then multiply by min unit to get real distance back
    float distOutside = (length(qNormalized) - 1.0) *  min(r.x, r.y);
    
    // this is just the harsh sdf clamp
    float distInside = min(max(q.x,q.y), 0.0);

    return distOutside + distInside;
}

inline bool outside_clips(float2 p, constant ClipUniform* clips, uint count) {
    float d = -1e20;

    for (uint i = 0; i < count; ++i) {
        ClipUniform clip = clips[i];
        d = max(d, rounded_rect_sdf(p - clip.rectCenter, clip.halfExtent, clip.cornerRadius));
    }

    return d > 0.0;
}
