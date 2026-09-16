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

struct ShadowUniform {
    float2 offset;
    float spread;
    float sigma;
    float4 color;
    uint inset;
};

inline float2 to_ndc(const float2 pt, float width = 512.0f, float height = 512.0f) {
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

inline CornerRadii spread_radii(CornerRadii radii, float growth) {
    float4 rx = float4(radii.topLeft.x, radii.topRight.x, radii.bottomRight.x, radii.bottomLeft.x);
    float4 ry = float4(radii.topLeft.y, radii.topRight.y, radii.bottomRight.y, radii.bottomLeft.y);

    float4 sx = 1.0;
    float4 sy = 1.0;

    if (growth > 0.0) {
        float4 tx = rx / growth - 1.0;
        float4 ty = ry / growth - 1.0;
        sx = select(1.0, 1.0 + tx * tx * tx, rx < growth);
        sy = select(1.0, 1.0 + ty * ty * ty, ry < growth);
    }

    rx = max(rx + growth * sx, 0.0);
    ry = max(ry + growth * sy, 0.0);

    CornerRadii result;
    result.topLeft = float2(rx.x, ry.x);
    result.topRight = float2(rx.y, ry.y);
    result.bottomRight = float2(rx.z, ry.z);
    result.bottomLeft = float2(rx.w, ry.w);
    return result;
}

// regular ass gaussian kernel
inline float gaussian(float distance, float sigma) {
    return exp(-0.5 * (distance / sigma) * (distance / sigma)) / (sigma * sqrt(2.0 * M_PI_F));
}

inline float shadow_coverage(float2 p, float2 halfExtent, CornerRadii radii, float sigma, float spread, float borderWidth) {
    halfExtent = max(halfExtent - borderWidth + spread, 0.0);
    radii = spread_radii(spread_radii(radii, -borderWidth), spread);
    float d = rounded_rect_sdf(p, halfExtent, radii);

    // harsh shadow handling
    if (sigma <= 0.0) {
        float px = fwidth(d);
        return clamp(0.5 - d / px, 0.0, 1.0);
    }

    // approx gaussian integral at -d / sigma
    // absurdly cool; thanks https://madebyevan.com/shaders/fast-rounded-rectangle-shadows/
    float x = abs(d) / (sigma * M_SQRT2_F);
    float denominator = 1.0 + (0.278393 + (0.230389 + 0.078108 * x * x) * x) * x;
    float tail = 0.5 / pow(denominator, 4.0);
    return d >= 0.0 ? tail : 1.0 - tail;
}
