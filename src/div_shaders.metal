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
    float2 screenPosition;
};

struct DivStyleUniforms {
    float4 color;
    CornerRadii cornerRadius;
    BorderUniform border;
    ShadowUniform shadow;
    float opacity;
};

struct DivGeometryUniforms {
    simd_float2 rectCenter;
    simd_float2 halfExtent;
    uint numClips;
    float3x3 transform;
};

struct DivUniforms {
    DivStyleUniforms style;
    DivGeometryUniforms geometry;
};

vertex DivVertexOut vertex_div(
   DivVertexIn in [[stage_in]],
   constant float2* offsets [[buffer(1)]],
   constant FrameInfo* frameInfo [[buffer(2)]],
   constant DivUniforms* uniforms [[buffer(3)]]
)
{
    DivVertexOut out;

    in.position += offsets[in.atom_id];

    float2 screenPosition = (uniforms->geometry.transform * float3(in.position, 1.0)).xy;

    float2 adjustedPosition = to_ndc(screenPosition, frameInfo->width, frameInfo->height);
    out.position = float4(adjustedPosition, 0.0, 1.0);
    out.worldPosition = float4(in.position, 0.0, 1.0);
    out.screenPosition = screenPosition;
    return out;
}

fragment float4 fragment_div(
    DivVertexOut in [[stage_in]],
    constant DivUniforms* uniforms [[buffer(0)]],
    constant ClipUniform* clips [[buffer(1)]]
)
{
    if (outside_clips(in.screenPosition, clips, uniforms->geometry.numClips)) {
        discard_fragment();
    }

    float2 localPosition = in.worldPosition.xy - uniforms->geometry.rectCenter;
    float d = rounded_rect_sdf(localPosition, uniforms->geometry.halfExtent, uniforms->style.cornerRadius);

    float px = fwidth(d);

    float outerMask = clamp(0.5 - d/px, 0.0, 1.0);

    float innerD = inset_rounded_rect_sdf(localPosition, uniforms->geometry.halfExtent, uniforms->style.cornerRadius, uniforms->style.border.widths);
    float innerMask = clamp(0.5 - innerD/px, 0.0, 1.0);

    float ringMask = clamp(outerMask - innerMask, 0.0, 1.0);
    float borderD = border_pattern(localPosition, uniforms->geometry.halfExtent, uniforms->style.cornerRadius, uniforms->style.border);
    float borderMask = ringMask * clamp(0.5 - borderD/px, 0.0, 1.0);
    float fillMask = outerMask;

    float4 fillColor = uniforms->style.color;
    float4 borderColor = uniforms->style.border.color;
    ShadowUniform shadow = uniforms->style.shadow;

    float outerCoverage = shadow_coverage(localPosition - shadow.offset, uniforms->geometry.halfExtent,
                                          uniforms->style.cornerRadius, shadow.sigma, shadow.spread, float4(0.0));
    float innerCoverage = shadow_coverage(localPosition - shadow.offset, uniforms->geometry.halfExtent,
                                          uniforms->style.cornerRadius, shadow.sigma, -shadow.spread,
                                          uniforms->style.border.widths);
    
    float outerShadowMask = outerCoverage * (1.0 - outerMask) * float(shadow.inset == 0);
    float insetShadowMask = (1.0 - innerCoverage) * innerMask * float(shadow.inset != 0);

    float3 premulFill = fillColor.rgb * fillColor.a;
    float fillAlpha = fillColor.a;

    float insetAlpha = shadow.color.a * insetShadowMask;
    float3 premulInset = (shadow.color.rgb * insetAlpha + premulFill * (1.0 - insetAlpha)) * fillMask;
    float insetOverFillAlpha = (insetAlpha + fillAlpha * (1.0 - insetAlpha)) * fillMask;

    float outerAlpha = shadow.color.a * outerShadowMask;
    float3 premulOuter = shadow.color.rgb * outerAlpha;

    float borderAlpha = borderColor.a * borderMask;
    float3 premulBorder = borderColor.rgb * borderAlpha;

    float alpha = borderAlpha + insetOverFillAlpha * (1.0 - borderAlpha) + outerAlpha;

    float3 rgb = premulBorder + premulInset * (1.0 - borderAlpha) + premulOuter;

    if (alpha > 1e-6) {
        rgb /= alpha;
    }

    return float4(rgb, alpha * uniforms->style.opacity);
}
