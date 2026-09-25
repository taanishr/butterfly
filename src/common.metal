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
    float3x3 inverseTransform;
};

enum class BorderStyle : uint {
    Solid = 0,
    Dashed = 1,
    Dotted = 2,
    Double = 3,
};

struct BorderUniform {
    float4 widths; // top, right, bottom, left
    float4 color;
    BorderStyle style;
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

inline bool outside_clips(float2 screenPosition, constant ClipUniform* clips, uint count) {
    float d = -1e20;

    for (uint i = 0; i < count; ++i) {
        ClipUniform clip = clips[i];
        float2 p = (clip.inverseTransform * float3(screenPosition, 1.0)).xy;
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

inline CornerRadii inset_radii(CornerRadii radii, float4 inset) {
    CornerRadii result;
    result.topLeft = max(radii.topLeft - float2(inset.w, inset.x), 0.0);
    result.topRight = max(radii.topRight - float2(inset.y, inset.x), 0.0);
    result.bottomRight = max(radii.bottomRight - float2(inset.y, inset.z), 0.0);
    result.bottomLeft = max(radii.bottomLeft - float2(inset.w, inset.z), 0.0);
    return result;
}

// the rect's center moves toward the thinner side when opposing insets differ
inline float2 inset_center(float4 inset) {
    return float2(inset.w - inset.y, inset.x - inset.z) / 2.0;
}

inline float2 inset_half_extent(float2 halfExtent, float4 inset) {
    return max(halfExtent - float2(inset.y + inset.w, inset.x + inset.z) / 2.0, 0.0);
}

inline float inset_rounded_rect_sdf(float2 pt, float2 halfExtent, CornerRadii radii, float4 inset) {
    return rounded_rect_sdf(pt - inset_center(inset), inset_half_extent(halfExtent, inset), inset_radii(radii, inset));
}

inline float circle_sdf(float2 pt, float2 center, float radius) {
    return length(pt - center) - radius;
}

inline float ellipse_arc_length(float2 radius, float angle) {
    const uint subintervals = 4;
    float step = angle / subintervals;
    float sum = 0.0;

    for (uint i = 0; i <= subintervals; ++i) {
        float theta = i * step;
        float weight = (i == 0 || i == subintervals) ? 1.0 : (i % 2 == 1 ? 4.0 : 2.0);
        float2 tangent = float2(radius.x * sin(theta), radius.y * cos(theta));
        sum += weight * length(tangent);
    }

    return sum * step / 3.0;
}

inline float ellipse_arc_angle(float2 radius, float arcLength, float quarterArcLength) {
    const uint iterations = 3;
    // initial guess; based on circle
    float angle = arcLength / quarterArcLength * M_PI_2_F;

    for (uint i = 0; i < iterations; ++i) {
        float2 tangent = float2(radius.x * sin(angle), radius.y * cos(angle));
        angle += (arcLength - ellipse_arc_length(radius, angle)) / length(tangent);
    }

    return angle;
}

// return 
inline float border_pattern(float2 pt, float d, float innerD, float2 halfExtent, CornerRadii radii, BorderUniform border) {
    float4 widths = border.widths;
    float width = max(max(widths.x, widths.y), max(widths.z, widths.w));

    if (width <= 0.0) {
        return 1e20;
    }

    float ring = max(d, -innerD);

    if (border.style == BorderStyle::Solid) {
        return ring;
    }

    if (border.style == BorderStyle::Double) {
        float outerThird = max(d, -inset_rounded_rect_sdf(pt, halfExtent, radii, widths / 3.0));
        float innerThird = max(inset_rounded_rect_sdf(pt, halfExtent, radii, 2.0 * widths / 3.0), -innerD);
        return min(outerThird, innerThird);
    }

    const float epsilon = 0.0001;

    // dash and gap ratios; switch based on dashed v dotted
    float dashLength = select(3.0 * width, width, border.style == BorderStyle::Dotted);
    float gapLength = select(2.0 * width, width, border.style == BorderStyle::Dotted);

    // figure out extent and corners of center line
    /*
    /---------- <- border edge
    |/--------- <- this is the center line
    |||-------- <- border edge
     ^
     |
     center line
     */
    float2 centerlineExtent = inset_half_extent(halfExtent, widths / 2.0);
    CornerRadii centerlineRadii = inset_radii(radii, widths / 2.0);
    pt -= inset_center(widths / 2.0);
    
    // compute the perimeter of the rounded rect (we compute the corners, arc lengths of corners, and the side lengths)
    float4 rx = max(float4(centerlineRadii.topLeft.x, centerlineRadii.topRight.x, centerlineRadii.bottomRight.x, centerlineRadii.bottomLeft.x), epsilon);
    float4 ry = max(float4(centerlineRadii.topLeft.y, centerlineRadii.topRight.y, centerlineRadii.bottomRight.y, centerlineRadii.bottomLeft.y), epsilon);
    float4 arc = float4(
        ellipse_arc_length(float2(rx.x, ry.x), M_PI_2_F),
        ellipse_arc_length(float2(rx.y, ry.y), M_PI_2_F),
        ellipse_arc_length(float2(rx.z, ry.z), M_PI_2_F),
        ellipse_arc_length(float2(rx.w, ry.w), M_PI_2_F)
    );
    float4 quadrantLength = centerlineExtent.x + centerlineExtent.y - rx - ry + arc;
    float perimeter = max(quadrantLength.x + quadrantLength.y + quadrantLength.z + quadrantLength.w, epsilon);

    // compute s; the distance *along the line*
    /*
        for each quadrant
        1. start with the previous qudrant lengths
        2. project onto the two sides + corner (parametric projection)
        3. find out which segment the point is closest to
        4. calculate that distance
        5.
     */
    float s = 0.0f;
    if (pt.y < 0.0 && pt.x < 0.0) {
        // side 1 projection
        float leftLength = centerlineExtent.y - ry.x;
        float leftAlong = clamp(-pt.y, 0.0, leftLength);
        float2 leftProjection = float2(-centerlineExtent.x, -leftAlong);
        float leftDistance = dot(pt - leftProjection, pt - leftProjection);

        // corner proj
        // get the angle (on the ellipse projected to circular space) then complete the projection
        float2 cornerCenter = float2(-centerlineExtent.x + rx.x, -centerlineExtent.y + ry.x);
        float2 cornerRadius = float2(rx.x, ry.x);
        float2 fromCenter = (pt - cornerCenter) / cornerRadius;
        float cornerAngle = clamp(atan2(-fromCenter.y, -fromCenter.x), 0.0, M_PI_2_F);
        float2 cornerProjection = cornerCenter - cornerRadius * float2(cos(cornerAngle), sin(cornerAngle));
        float cornerDistance = dot(pt - cornerProjection, pt - cornerProjection);

        // side 2 projection
        float topLength = centerlineExtent.x - rx.x;
        float topAlong = clamp(pt.x + centerlineExtent.x - rx.x, 0.0, topLength);
        float2 topProjection = float2(-centerlineExtent.x + rx.x + topAlong, -centerlineExtent.y);
        float topDistance = dot(pt - topProjection, pt - topProjection);

        float bestDistance = leftDistance;
        s = leftAlong;
        if (cornerDistance < bestDistance) {
            bestDistance = cornerDistance;
            // because the angle is in elliptical space
            // we need to compute an integral to get how far this arc length is
            // lmfao. calculus 3!!!
            s = leftLength + ellipse_arc_length(cornerRadius, cornerAngle);
        }
        
        if (topDistance < bestDistance) {
            s = leftLength + arc.x + topAlong;
        }
    } else if (pt.y < 0.0) {
        float distanceToSegmentStart = quadrantLength.x;

        float topLength = centerlineExtent.x - rx.y;
        float topAlong = clamp(pt.x, 0.0, topLength);
        float2 topProjection = float2(topAlong, -centerlineExtent.y);
        float topDistance = dot(pt - topProjection, pt - topProjection);

        float2 cornerCenter = float2(centerlineExtent.x - rx.y, -centerlineExtent.y + ry.y);
        float2 cornerRadius = float2(rx.y, ry.y);
        float2 fromCenter = (pt - cornerCenter) / cornerRadius;
        float cornerAngle = clamp(atan2(fromCenter.x, -fromCenter.y), 0.0, M_PI_2_F);
        float2 cornerProjection = cornerCenter + cornerRadius * float2(sin(cornerAngle), -cos(cornerAngle));
        float cornerDistance = dot(pt - cornerProjection, pt - cornerProjection);

        float rightLength = centerlineExtent.y - ry.y;
        float rightAlong = clamp(pt.y + centerlineExtent.y - ry.y, 0.0, rightLength);
        float2 rightProjection = float2(centerlineExtent.x, -centerlineExtent.y + ry.y + rightAlong);
        float rightDistance = dot(pt - rightProjection, pt - rightProjection);

        float bestDistance = topDistance;
        s = distanceToSegmentStart + topAlong;
        if (cornerDistance < bestDistance) {
            bestDistance = cornerDistance;
            s = distanceToSegmentStart + topLength + ellipse_arc_length(cornerRadius.yx, cornerAngle);
        }
        
        if (rightDistance < bestDistance) {
            s = distanceToSegmentStart + topLength + arc.y + rightAlong;
        }
    } else if (pt.x >= 0.0) {
        float distanceToSegmentStart = quadrantLength.x + quadrantLength.y;

        float rightLength = centerlineExtent.y - ry.z;
        float rightAlong = clamp(pt.y, 0.0, rightLength);
        float2 rightProjection = float2(centerlineExtent.x, rightAlong);
        float rightDistance = dot(pt - rightProjection, pt - rightProjection);

        float2 cornerCenter = float2(centerlineExtent.x - rx.z, centerlineExtent.y - ry.z);
        float2 cornerRadius = float2(rx.z, ry.z);
        float2 fromCenter = (pt - cornerCenter) / cornerRadius;
        float cornerAngle = clamp(atan2(fromCenter.y, fromCenter.x), 0.0, M_PI_2_F);
        float2 cornerProjection = cornerCenter + cornerRadius * float2(cos(cornerAngle), sin(cornerAngle));
        float cornerDistance = dot(pt - cornerProjection, pt - cornerProjection);

        float bottomLength = centerlineExtent.x - rx.z;
        float bottomAlong = clamp(centerlineExtent.x - rx.z - pt.x, 0.0, bottomLength);
        float2 bottomProjection = float2(centerlineExtent.x - rx.z - bottomAlong, centerlineExtent.y);
        float bottomDistance = dot(pt - bottomProjection, pt - bottomProjection);

        float bestDistance = rightDistance;
        s = distanceToSegmentStart + rightAlong;
        if (cornerDistance < bestDistance) {
            bestDistance = cornerDistance;
            s = distanceToSegmentStart + rightLength + ellipse_arc_length(cornerRadius, cornerAngle);
        }
        
        if (bottomDistance < bestDistance) {
            s = distanceToSegmentStart + rightLength + arc.z + bottomAlong;
        }
    } else {
        float distanceToSegmentStart = quadrantLength.x + quadrantLength.y + quadrantLength.z;

        float bottomLength = centerlineExtent.x - rx.w;
        float bottomAlong = clamp(-pt.x, 0.0, bottomLength);
        float2 bottomProjection = float2(-bottomAlong, centerlineExtent.y);
        float bottomDistance = dot(pt - bottomProjection, pt - bottomProjection);

        float2 cornerCenter = float2(-centerlineExtent.x + rx.w, centerlineExtent.y - ry.w);
        float2 cornerRadius = float2(rx.w, ry.w);
        float2 fromCenter = (pt - cornerCenter) / cornerRadius;
        float cornerAngle = clamp(atan2(-fromCenter.x, fromCenter.y), 0.0, M_PI_2_F);
        float2 cornerProjection = cornerCenter + cornerRadius * float2(-sin(cornerAngle), cos(cornerAngle));
        float cornerDistance = dot(pt - cornerProjection, pt - cornerProjection);

        float leftLength = centerlineExtent.y - ry.w;
        float leftAlong = clamp(centerlineExtent.y - ry.w - pt.y, 0.0, leftLength);
        float2 leftProjection = float2(-centerlineExtent.x, centerlineExtent.y - ry.w - leftAlong);
        float leftDistance = dot(pt - leftProjection, pt - leftProjection);

        float bestDistance = bottomDistance;
        s = distanceToSegmentStart + bottomAlong;
        if (cornerDistance < bestDistance) {
            bestDistance = cornerDistance;
            s = distanceToSegmentStart + bottomLength + ellipse_arc_length(cornerRadius.yx, cornerAngle);
        }
        
        if (leftDistance < bestDistance) {
            s = distanceToSegmentStart + bottomLength + arc.w + leftAlong;
        }
    }

    float period = perimeter / max(1.0, round(perimeter / (dashLength + gapLength)));
    float nearest = round(s / period);

    // if dashed; job done, just return the period calc
    if (border.style == BorderStyle::Dashed) {
        float dash = abs(s - nearest * period) - dashLength / 2.0;
        return max(ring, dash);
    }

    // now we have to compute the dots
    /*
        we're basically converting the distance along the projection
        into a distance from the nearest dot
     */
    float nearestDot = 1e20;

    // check left side, middle & right side of dot position to figure out who the nearest dot is
    // this really isnt that hard to understand its just v long because of the quadrant checks
    for (int i = -1; i <= 1; ++i) {
        float t = (nearest + i) * period;
        t -= perimeter * floor(t / perimeter);
        float2 center;

        if (t < quadrantLength.x) {
            float leftLength = centerlineExtent.y - ry.x;
            float2 cornerCenter = float2(-centerlineExtent.x + rx.x, -centerlineExtent.y + ry.x);
            float2 cornerRadius = float2(rx.x, ry.x);

            if (t < leftLength) {
                center = float2(-centerlineExtent.x, -t);
            } else if (t < leftLength + arc.x) {
                float cornerAngle = ellipse_arc_angle(cornerRadius, t - leftLength, arc.x);
                center = cornerCenter - cornerRadius * float2(cos(cornerAngle), sin(cornerAngle));
            } else {
                center = float2(-centerlineExtent.x + rx.x + t - leftLength - arc.x, -centerlineExtent.y);
            }
        } else if (t < quadrantLength.x + quadrantLength.y) {
            t -= quadrantLength.x;
            float topLength = centerlineExtent.x - rx.y;
            float2 cornerCenter = float2(centerlineExtent.x - rx.y, -centerlineExtent.y + ry.y);
            float2 cornerRadius = float2(rx.y, ry.y);

            if (t < topLength) {
                center = float2(t, -centerlineExtent.y);
            } else if (t < topLength + arc.y) {
                float cornerAngle = ellipse_arc_angle(cornerRadius.yx, t - topLength, arc.y);
                center = cornerCenter + cornerRadius * float2(sin(cornerAngle), -cos(cornerAngle));
            } else {
                center = float2(centerlineExtent.x, -centerlineExtent.y + ry.y + t - topLength - arc.y);
            }
        } else if (t < quadrantLength.x + quadrantLength.y + quadrantLength.z) {
            t -= quadrantLength.x + quadrantLength.y;
            float rightLength = centerlineExtent.y - ry.z;
            float2 cornerCenter = float2(centerlineExtent.x - rx.z, centerlineExtent.y - ry.z);
            float2 cornerRadius = float2(rx.z, ry.z);

            if (t < rightLength) {
                center = float2(centerlineExtent.x, t);
            } else if (t < rightLength + arc.z) {
                float cornerAngle = ellipse_arc_angle(cornerRadius, t - rightLength, arc.z);
                center = cornerCenter + cornerRadius * float2(cos(cornerAngle), sin(cornerAngle));
            } else {
                center = float2(centerlineExtent.x - rx.z - t + rightLength + arc.z, centerlineExtent.y);
            }
        } else {
            t -= quadrantLength.x + quadrantLength.y + quadrantLength.z;
            float bottomLength = centerlineExtent.x - rx.w;
            float2 cornerCenter = float2(-centerlineExtent.x + rx.w, centerlineExtent.y - ry.w);
            float2 cornerRadius = float2(rx.w, ry.w);

            if (t < bottomLength) {
                center = float2(-t, centerlineExtent.y);
            } else if (t < bottomLength + arc.w) {
                float cornerAngle = ellipse_arc_angle(cornerRadius.yx, t - bottomLength, arc.w);
                center = cornerCenter + cornerRadius * float2(-sin(cornerAngle), cos(cornerAngle));
            } else {
                center = float2(-centerlineExtent.x, centerlineExtent.y - ry.w - t + bottomLength + arc.w);
            }
        }

        nearestDot = min(nearestDot, circle_sdf(pt, center, width / 2.0));
    }

    return max(ring, nearestDot);
}

inline float shadow_coverage(float2 p, float2 halfExtent, CornerRadii radii, float sigma, float spread, float4 borderWidths) {
    p -= inset_center(borderWidths);
    halfExtent = max(inset_half_extent(halfExtent, borderWidths) + spread, 0.0);
    radii = spread_radii(inset_radii(radii, borderWidths), spread);
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
