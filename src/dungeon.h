#pragma once

#include "math.h"

#define FLOOR_WIDTH 9
#define FLOOR_HEIGHT 9

enum Direction {
    DirectionLeft = 0,
    DirectionUp = 1,
    DirectionRight = 2,
    DirectionDown = 3,
};

struct RoomLink {

    enum Direction door_dir;
};

typedef struct FloorPlan {
    bool rooms[FLOOR_WIDTH][FLOOR_HEIGHT];
    Vector2i boss;
    Vector2i item;
} FloorPlan;

/// @brief generate a floor plan with branching paths
/// @param floor_plan the target floor plan
/// @param floor_depth the current "level" the player is one, should be at least 1. affects how many rooms are spawned.
/// @return true if the plan successfully generated, false otherwise
bool floor_plan_generate(FloorPlan* floor_plan, int floor_depth);

bool floor_has_room_dir(FloorPlan* floor_plan, Vector2i cell, enum Direction direction);
