#include "math.h"
#include "dungeon.h"

void* memset(void *dest, int val, size_t size);

void floor_plan_generate(FloorPlan* floor_plan) {
    *floor_plan = (FloorPlan){0};
    // center
    floor_plan->rooms[2][2] = true;
    // right of center
    floor_plan->rooms[3][2] = true;
    // above center
    floor_plan->rooms[2][1] = true;
    floor_plan->rooms[2][3] = true;
    floor_plan->rooms[1][2] = true;
}

bool floor_has_room_dir(FloorPlan* floor_plan, Vector2i cell, enum Direction direction) {
    if (cell.x <= 0 || cell.x >= FLOOR_WIDTH - 1 || cell.y <= 0 || cell.x >= FLOOR_HEIGHT -1)
        return false;

    switch (direction) {
    case DirectionLeft:
        cell.x -= 1;
        break;
    case DirectionRight:
        cell.x += 1;
        break;
    case DirectionUp:
        cell.y -= 1;
        break;
    case DirectionDown:
        cell.y += 1;
        break;
    }

    return floor_plan->rooms[cell.x][cell.y];
}
