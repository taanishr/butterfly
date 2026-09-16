//
//  shell_shaders.metal
//  gui
//
//  Created by Taanish Reja on 8/20/25.
//

#include <metal_stdlib>
#include "common.metal"

using namespace metal;

struct DivVertexIn {
    float2 position [[attribute(0)]];
    unsigned int atom_id [[attribute(1)]];
};

struct DivVertexOut {
    float4 position [[position]];
    float4 worldPosition;
};

struct DivStyleUniforms {
    float4 color;
    CornerRadii cornerRadius;
    float borderWidth;
    float4 borderColor;
    ShadowUniform shadow;
};

struct DivGeometryUniforms {
    simd_float2 rectCenter;
    simd_float2 halfExtent;
    uint numClips;
};

struct DivUniforms {
    DivStyleUniforms style;
    DivGeometryUniforms geometry;
};

vertex DivVertexOut vertex_div(
   DivVertexIn in [[stage_in]],
   constant float2* offsets [[buffer(1)]],
   constant FrameInfo* frameInfo [[buffer(2)]]
)
{
    DivVertexOut out;
    
    in.position += offsets[in.atom_id];
    
    float2 adjustedPosition = to_ndc(in.position, frameInfo->width, frameInfo->height);
    out.position = float4(adjustedPosition, 0.0, 1.0);
    out.worldPosition = float4(in.position, 0.0, 1.0);
    return out;
}

fragment float4 fragment_div(
    DivVertexOut in [[stage_in]],
    constant DivUniforms* uniforms [[buffer(0)]],
    constant ClipUniform* clips [[buffer(1)]]
)
{
    if (outside_clips(in.worldPosition.xy, clips, uniforms->geometry.numClips)) {
        discard_fragment();
    }

    float2 localPosition = in.worldPosition.xy - uniforms->geometry.rectCenter;
    float d = rounded_rect_sdf(localPosition, uniforms->geometry.halfExtent, uniforms->style.cornerRadius);

    float px = fwidth(d);

    float outerMask = clamp(0.5 - d/px, 0.0, 1.0);
    
    float innerD = d + uniforms->style.borderWidth;
    float innerMask = clamp(0.5 - innerD/px, 0.0, 1.0);
    
    float borderMask = outerMask - innerMask;
    float fillMask = innerMask;

    float4 fillColor = uniforms->style.color;
    float4 borderColor = uniforms->style.borderColor;
    ShadowUniform shadow = uniforms->style.shadow;

    float outerCoverage = shadow_coverage(localPosition - shadow.offset, uniforms->geometry.halfExtent,
                                          uniforms->style.cornerRadius, shadow.sigma, shadow.spread, 0.0);
    float innerCoverage = shadow_coverage(localPosition - shadow.offset, uniforms->geometry.halfExtent,
                                          uniforms->style.cornerRadius, shadow.sigma, -shadow.spread,
                                          uniforms->style.borderWidth);
    
    float outerShadowMask = outerCoverage * (1.0 - outerMask) * float(shadow.inset == 0);
    float insetShadowMask = (1.0 - innerCoverage) * float(shadow.inset != 0);

    float3 premulFill = fillColor.rgb * fillColor.a;
    float fillAlpha = fillColor.a;

    float insetAlpha = shadow.color.a * insetShadowMask;
    float3 premulInset = (shadow.color.rgb * insetAlpha + premulFill * (1.0 - insetAlpha)) * fillMask;
    float insetOverFillAlpha = (insetAlpha + fillAlpha * (1.0 - insetAlpha)) * fillMask;

    float outerAlpha = shadow.color.a * outerShadowMask;
    float3 premulOuter = shadow.color.rgb * outerAlpha;

    float3 premulBorder = borderColor.rgb * borderColor.a * borderMask;

    float alpha = borderMask * borderColor.a + insetOverFillAlpha + outerAlpha;

    float3 rgb = premulInset + premulOuter + premulBorder;

    if (alpha > 1e-6) {
        rgb /= alpha;
    }

    return float4(rgb, alpha);
}
