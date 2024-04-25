#pragma once

#include "math.h"
#include "genarray.h"

#define MAX_ENTITIES 1024

// game.h forward decls
typedef struct GameData GameData;
typedef struct GameState GameState;

#define CELL_SIZE 8

typedef struct Entity {
    union {
        Handle handle;
        size_t handle_value;
    };

    // position on tile grid
    Vector2i cpos;
    // fraction from 0 to 1 representing sub-grid position
    Vector2 fpos;

    // velocity in cells per second
    Vector2 velocity;
    // friction multiplier applied to velocity every frame, after velocity is applied to position
    float normal_friction;
    // amount of time it should take to reach normal_max_speed - used to calculate per-frame
    // acceleration
    float normal_accel_time;
    // max running speed, entities will accelerate up to this speed
    float normal_max_speed;

    // AABB collision info used for collisions with tiles, units are in cells
    Rectangle c_aabb;
    // Radius collision info used for character pushing and bullet intersections
    Vector2 c_radial_center;
    float c_radius;
    bool c_pushes;

    int health;
    int max_health;
} Entity;

GENARRAY_DEFINE(Entity, MAX_ENTITIES, ent)
typedef struct EntityArray EntityArray;

static inline Vector2 get_pixel_pos(const Entity* entity) {
    Vector2 pos = {
        (entity->cpos.x + entity->fpos.x) * CELL_SIZE,
        (entity->cpos.y + entity->fpos.y) * CELL_SIZE
    };
    return pos;
}

static inline void set_pixel_pos(Entity* entity, Vector2 pos) {
    entity->cpos.x = (int)(pos.x / CELL_SIZE);
    entity->cpos.y = (int)(pos.y / CELL_SIZE);
    entity->fpos.x = fmodf(pos.x / CELL_SIZE, 1.0f);
    entity->fpos.y = fmodf(pos.y / CELL_SIZE, 1.0f);
}

void draw_player(GameState* state);
void draw_box(GameState* state); // NOTE: test function
void ent_move(GameState* state, Entity* entity);
void ent_repel_ent(Entity* self, Entity* other);

// enumerate each occupied entry in an EntityArrayEntry - assumes that genarrays are always fully initialized to their max capacity
#define ENT_ARRAY_FOREACH(arr, iter) for (struct EntityArrayEntry* iter = &(arr).data[0]; iter != &(arr).data[MAX_ENTITIES-1]; iter++) if (iter->occupied)
#define ENT_REF_ARRAY_FOREACH(arr, iter) for (struct EntityArrayEntry* iter = &(arr)->data[0]; iter != &(arr)->data[MAX_ENTITIES-1]; iter++) if (iter->occupied)
