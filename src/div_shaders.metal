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
    uint instance_id [[flat]];
};

struct DivUniforms {
    float4 color;
    CornerRadii cornerRadius;
    BorderUniform border;
    ShadowUniform shadow;
    float opacity;
};

struct DivGeometry {
    simd_float2 rectCenter;
    simd_float2 halfExtent;
    simd_float2 quadMin;
    simd_float2 quadMax;
    CornerRadii cornerRadius;
    float4 borderWidths;
    uint numClips;
    float3x3 transform;
};

vertex DivVertexOut vertex_div(
   DivVertexIn in [[stage_in]],
   uint instance_id [[instance_id]],
   constant float2* offsets [[buffer(1)]],
   constant FrameInfo* frameInfo [[buffer(2)]],
   constant DivGeometry* geometry [[buffer(3)]]
)
{
    DivVertexOut out;

    DivGeometry g = geometry[instance_id];

    float2 position = mix(g.quadMin, g.quadMax, in.position) + offsets[instance_id];

    float2 screenPosition = (g.transform * float3(position, 1.0)).xy;

    float2 adjustedPosition = to_ndc(screenPosition, frameInfo->width, frameInfo->height);
    out.position = float4(adjustedPosition, 0.0, 1.0);
    out.worldPosition = float4(position, 0.0, 1.0);
    out.screenPosition = screenPosition;
    out.instance_id = instance_id;
    return out;
}

fragment float4 fragment_div(
    DivVertexOut in [[stage_in]],
    constant DivUniforms* uniforms [[buffer(0)]],
    constant ClipUniform* clips [[buffer(1)]],
    constant DivGeometry* geometry [[buffer(2)]]
)
{
    DivGeometry g = geometry[in.instance_id];

    if (outside_clips(in.screenPosition, clips, g.numClips)) {
        discard_fragment();
    }

    BorderUniform border = uniforms->border;
    border.widths = g.borderWidths;

    float2 localPosition = in.worldPosition.xy - g.rectCenter;
    float d = rounded_rect_sdf(localPosition, g.halfExtent, g.cornerRadius);

    float px = fwidth(d);

    float outerMask = clamp(0.5 - d/px, 0.0, 1.0);

    float innerD = inset_rounded_rect_sdf(localPosition, g.halfExtent, g.cornerRadius, border.widths);
    float innerMask = clamp(0.5 - innerD/px, 0.0, 1.0);

    float ringMask = clamp(outerMask - innerMask, 0.0, 1.0);
    float borderD = border_pattern(localPosition, g.halfExtent, g.cornerRadius, border);
    float borderMask = ringMask * clamp(0.5 - borderD/px, 0.0, 1.0);
    float fillMask = outerMask;

    float4 fillColor = uniforms->color;
    float4 borderColor = border.color;
    ShadowUniform shadow = uniforms->shadow;

    float outerCoverage = shadow_coverage(localPosition - shadow.offset, g.halfExtent,
                                          g.cornerRadius, shadow.sigma, shadow.spread, float4(0.0));
    float innerCoverage = shadow_coverage(localPosition - shadow.offset, g.halfExtent,
                                          g.cornerRadius, shadow.sigma, -shadow.spread,
                                          border.widths);
    
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

    return float4(rgb, alpha * uniforms->opacity);
}
