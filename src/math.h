#pragma once

// forward decl to avoid including stdlib math.h
float fmodf(float x, float y);

typedef struct Vector2i {
    int x;
    int y;
} Vector2i;
