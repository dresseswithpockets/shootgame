#pragma once

#include <limits.h>

// forward decl to avoid including stdlib math.h
float fmodf(float x, float y);

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

static inline int signi(float x) {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}
