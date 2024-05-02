#pragma once

#include <limits.h>

DLLIMPORT float __cdecl fmodf(float x, float y);

typedef struct Vector2i {
    int x;
    int y;
} Vector2i;

#define ARRAY_LEN(arr) (sizeof(arr)/sizeof(arr[0]))
#define HAS_FLAG(value, flag) (((value) & (flag)) == (flag))

static inline Vector2 v2itof(Vector2i vector) {
    return (Vector2) {(float)vector.x, (float)vector.y};
}

static inline Vector2i v2ftoi(Vector2 vector) {
    return (Vector2i) {(int)vector.x, (int)vector.y};
}

static inline Vector2 vector2_subpixel(Vector2 pos) {
    double integral;
    return (Vector2) { modf(pos.x, &integral), modf(pos.y, &integral) };
}

static inline Vector2i vector2i_lerp(Vector2i a, Vector2i b, float t) {
    return v2ftoi(Vector2Lerp(v2itof(a), v2itof(b), t));
}

static inline bool vector2i_eq(Vector2i a, Vector2i b) {
    return a.x == b.x && a.y == b.y;
}

static inline Vector2 vector2_polar(float length, float theta) {
    Vector2 result = {
        cosf(theta) * length,
        sinf(theta) * length,
    };
    return result;
}

static inline float rand_float(float min, float max) {
    return ((float)GetRandomValue(0, INT_MAX)/(float)(INT_MAX)) * (max - min) + min;
}

static inline float smoothdamp(float from, float to, float lambda, float dt)
{
    return Lerp(from, to, 1.0f - expf(-lambda * dt));
}

static inline float short_angle_dist(float from, float to) {
    float max_angle = PI * 2;
    float difference = fmodf(to - from, max_angle);
    return fmodf(2 * difference, max_angle) - difference;
}

static inline float smoothdamp_angle(float from, float to, float lambda, float dt) {
    return smoothdamp(from, from + short_angle_dist(from, to), lambda, dt);
}

static inline float lerp_angle(float from, float to, float t) {
    return from + short_angle_dist(from, to) * t;
}

static inline float move_towards(float value, float to, float max) {
    if (fabsf(to - value) <= max) return to;
    if (to < value) return value - max;
    return value + max;
}

static inline int signi(float x) {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}
