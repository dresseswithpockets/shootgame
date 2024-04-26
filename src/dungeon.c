#include <assert.h>
#include <stdlib.h>
#include "math.h"
#include "dungeon.h"

void* memset(void *dest, int val, size_t size);

struct Vector2iStack {
    Vector2i stack[FLOOR_WIDTH * FLOOR_HEIGHT];
    int stack_len;
};

void clear_stack(struct Vector2iStack* stack) {
    stack->stack_len = 0;
}

void push_back(struct Vector2iStack* stack, Vector2i value) {
    assert(stack->stack_len < ARRAY_LEN(stack->stack) - 1);
    stack->stack[stack->stack_len] = value;
    stack->stack_len += 1;
}

void remove(struct Vector2iStack* stack, int index) {
    assert(index < stack->stack_len);

    stack->stack_len -= 1;
    for (int i = index; i < stack->stack_len; i++) {
        stack->stack[i] = stack->stack[i + 1];
    }
}

Vector2i pop_back(struct Vector2iStack* stack) {
    assert(stack->stack_len > 0);
    stack->stack_len -= 1;
    return stack->stack[stack->stack_len];
}

int count_neighbors(FloorPlan* floor_plan, int x, int y) {
    int count = 0;
    if (x > 0 && floor_plan->rooms[x-1][y]) count += 1;
    if (x < FLOOR_WIDTH - 1 && floor_plan->rooms[x+1][y]) count += 1;
    if (y > 0 && floor_plan->rooms[x][y-1]) count += 1;
    if (y < FLOOR_HEIGHT - 1 && floor_plan->rooms[x][y+1]) count += 1;
    return count;
}

bool check_adjacent(FloorPlan* floor_plan, int x, int y, bool require_low_neighbors) {
    if (x < 0 || x >= FLOOR_WIDTH || y < 0 || y >= FLOOR_HEIGHT) return false;
    if (floor_plan->rooms[x][y]) return false;
    if (require_low_neighbors && count_neighbors(floor_plan, x, y) > 1) return false;
    return true;
}

int compare_distance(const void* a, const void* b) {
    Vector2 center = (Vector2) { 2, 2 };
    float distance_a = Vector2DistanceSqr(v2itof(*(Vector2i*)a), center);
    float distance_b = Vector2DistanceSqr(v2itof(*(Vector2i*)b), center);
    if (distance_a < distance_b) return 1;
    if (distance_a > distance_b) return -1;
    return 0;
}

void floor_plan_generate(FloorPlan* floor_plan, int floor_depth) {
    *floor_plan = (FloorPlan){0};

    // min_room_count does not include the center room, which is always present
    int min_room_count = (int)(2.0 * floor_depth + GetRandomValue(3, 4));
    int min_branch_length = min_room_count / 4;
    // must have at least 1 dead end for each: boss room & item room
    int min_dead_ends = 2;

    struct Vector2iStack room_stack = {0};
    Vector2i center_idx = (Vector2i) { 2, 2 };

    // center is always enabled
    floor_plan->rooms[2][2] = true;
    int room_count = 1;
    while (room_count < min_room_count) {
        // if the center room has too many neighbors, then break out of the loop and scan for new
        // potential cells, instead of pushing the center to the stack.
        int center_neighbors = count_neighbors(floor_plan, center_idx.x, center_idx.y);
        if (center_neighbors == 4) {
            break;
        }

        // if we ran out of rooms in the stack, then default to going back to the center room for a new branch
        push_back(&room_stack, center_idx);

        int current_branch_length = 0;
        while (room_stack.stack_len > 0) {
            if (room_count >= min_room_count) break;

            Vector2i next_idx = pop_back(&room_stack);

            // there is a N-in-8 chance (N is branch length) that no adjacent cell is picked, unless:
            // - the current cell is the center cell
            // - or, the center cell has too many neighbors
            // - or, this branch isn't big enough
            if (!vector2i_eq(center_idx, next_idx) && current_branch_length >= min_branch_length) {
                center_neighbors = count_neighbors(floor_plan, center_idx.x, center_idx.y);
                // the chance
                // chance to skip scales with the length of the branch
                if (center_neighbors < 4 && GetRandomValue(0, 8) <= current_branch_length) {
                    // met all the criteria to skip the current cell and go back to the center cell
                    break;
                }
            }

            struct Vector2iStack potential_next_rooms = {0};
            if (check_adjacent(floor_plan, next_idx.x - 1, next_idx.y, !vector2i_eq(next_idx, center_idx))) {
                push_back(&potential_next_rooms, (Vector2i) { next_idx.x - 1, next_idx.y });
            }
            if (check_adjacent(floor_plan, next_idx.x + 1, next_idx.y, !vector2i_eq(next_idx, center_idx))) {
                push_back(&potential_next_rooms, (Vector2i) { next_idx.x + 1, next_idx.y });
            }
            if (check_adjacent(floor_plan, next_idx.x, next_idx.y - 1, !vector2i_eq(next_idx, center_idx))) {
                push_back(&potential_next_rooms, (Vector2i) { next_idx.x, next_idx.y - 1 });
            }
            if (check_adjacent(floor_plan, next_idx.x, next_idx.y + 1, !vector2i_eq(next_idx, center_idx))) {
                push_back(&potential_next_rooms, (Vector2i) { next_idx.x, next_idx.y + 1 });
            }

            // this branch's tail is stuck in a corner surrounded by cells already in use or OOB
            if (potential_next_rooms.stack_len == 0) {
                break;
            }

            next_idx = potential_next_rooms.stack[GetRandomValue(0, potential_next_rooms.stack_len - 1)];
            push_back(&room_stack, next_idx);
            floor_plan->rooms[next_idx.x][next_idx.y] = true;
            current_branch_length += 1;
            room_count += 1;
        }
    }

    // TODO: scan for new rooms to add if we dont have enough? alternatively, prioritize dead ends in this rare case
    // int allowed_neighbor_count = 1;
    // while (room_count < min_room_count && allowed_neighbor_count < 5) {
    //     // find all cells with few neighbors
    // }

    // query the floor plan for all actual dead ends and potential dead end indices
    struct Vector2iStack real_dead_ends = {0};
    struct Vector2iStack potential_dead_ends = {0};
    for (int x = 0; x < FLOOR_WIDTH; x++) {
        for (int y = 0; y < FLOOR_HEIGHT; y++) {
            Vector2i room_idx = (Vector2i){ x, y };
            if (count_neighbors(floor_plan, x, y) == 1 && !vector2i_eq(room_idx, center_idx)) {
                if (floor_plan->rooms[x][y]) {
                    push_back(&real_dead_ends, room_idx);
                } else {
                    push_back(&potential_dead_ends, room_idx);
                }
            }
        }
    }

    // if there is a dead end deficit, then we will randomly select new ones to add
    int deficit = min_dead_ends - real_dead_ends.stack_len;
    assert(potential_dead_ends.stack_len >= deficit && "we hit the maximum deadends possible in the floor plan 😵‍💫😵‍💫");
    for (int i = 0; i < deficit; i++) {
        int index = GetRandomValue(0, potential_dead_ends.stack_len - 1);
        push_back(&real_dead_ends, potential_dead_ends.stack[index]);
        remove(&potential_dead_ends, index);
    }

    // sort the real dead ends by distance from center, and pick the furthest for special rooms
    assert(real_dead_ends.stack_len >= min_dead_ends);
    qsort(&real_dead_ends.stack, real_dead_ends.stack_len, sizeof(Vector2i), compare_distance);
    floor_plan->boss = real_dead_ends.stack[0];
    floor_plan->item = real_dead_ends.stack[1];
}

bool floor_has_room_dir(FloorPlan* floor_plan, Vector2i cell, enum Direction direction) {
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

    if (cell.x < 0 || cell.x > FLOOR_WIDTH - 1 || cell.y < 0 || cell.x > FLOOR_HEIGHT - 1)
        return false;

    return floor_plan->rooms[cell.x][cell.y];
}
