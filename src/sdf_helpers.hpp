//
//  sdf_helpers.hpp
//  gui
//
//  Created by Taanish Reja on 8/24/25.
//

#pragma once
#include <cmath>
#include "metal_imports.hpp"
#include "layout/style.hpp"

float rounded_rect_sdf(simd_float2 pt, simd_float2 halfExtent, const style::CornerRadii& radii);
