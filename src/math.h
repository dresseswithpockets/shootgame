#pragma once

#include "raylib.h"

static inline double lerp(double a, double b, double t) {
    return a * (1.0 - t) + (b * t);
}

static inline Vector2 lerp_vec(Vector2 a, Vector2 b, double t) {
    Vector2 result = {
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t)
    };
    return result;
}