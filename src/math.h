#pragma once

#include <limits.h>

// forward decl to avoid including stdlib math.h
float fmodf(float x, float y);

typedef struct Vector2i {
    int x;
    int y;
} Vector2i;

#define ARRAY_LEN(arr) (sizeof(arr)/sizeof(arr[0]))

static inline float rand_float(float min, float max) {
    return ((float)GetRandomValue(0, INT_MAX)/(float)(INT_MAX)) * (max - min) + min;
}

static inline int signi(float x) {
    if (x < 0) return -1;
    if (x > 0) return 1;
    return 0;
}
