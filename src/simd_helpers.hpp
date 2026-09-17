#pragma once
#include <simd/simd.h>

inline simd_float3x3 composeTransform(simd_float2 translate, float rotate, simd_float2 scale) {
    float c = std::cos(rotate);
    float s = std::sin(rotate);

    simd_float3x3 m = matrix_identity_float3x3;
    m.columns[0] = {c * scale.x, s * scale.x, 0.0f};
    m.columns[1] = {-s * scale.y, c * scale.y, 0.0f};
    m.columns[2] = {translate.x, translate.y, 1.0f};
    return m;
}
