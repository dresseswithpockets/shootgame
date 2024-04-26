#pragma once

#include "math.h"
#include "genarray.h"
#include "sprite.h"

// since entities are shared across rooms, we have a pretty large limit. in practice, this can
// probably be way lower. I will have to do some testing to see what the reasonable limit is.
#define MAX_ENTITIES 8192

// game.h forward decls
typedef struct GameData GameData;
typedef struct GameState GameState;

#define CELL_SIZE 8

enum CollisionFlag {
    CollisionFlagLeft = 1,
    CollisionFlagUp = 1 << 1,
    CollisionFlagRight = 1 << 2,
    CollisionFlagDown = 1 << 3,
};

#define COLLISION_FLAG_DIR_MASK 0b00001111
#define HAS_COLLISION(flag) ((flag) & COLLISION_FLAG_DIR_MASK)

typedef struct Entity {
    union {
        Handle handle;
        size_t handle_value;
    };

    // integer position in pixels
    Vector2i pos_pixel;
    // subpixel position remainder
    Vector2 pos_subpixel;
    // total position calculated after moving pos_pixel or pos_subpixel
    Vector2 pos;

    // velocity in cells per second
    Vector2 velocity;
    // friction multiplier applied to velocity every frame, after velocity is applied to position
    float normal_friction;
    // amount of time it should take to reach normal_max_speed - used to calculate per-frame
    // acceleration
    float normal_accel_time;
    // max running speed, entities will accelerate up to this speed
    float normal_max_speed;

    Sprite* sprite;

    // AABB collision info used for collisions with tiles, units are in cells
    Vector2i c_size;
    enum CollisionFlag c_flags;

    // Radius collision info used for character pushing and bullet intersections, units are in cells
    Vector2 c_radial_center;
    float c_radius;
    bool c_pushes;

    int health;
    int max_health;
} Entity;

GENARRAY_DEFINE(Entity, MAX_ENTITIES, ent)
typedef struct EntityArray EntityArray;

static inline Vector2i get_cell_pos(const Entity* entity) {
    Vector2i pos = {
        (int)(entity->pos.x / (float)CELL_SIZE),
        (int)(entity->pos.y / (float)CELL_SIZE)
    };
    return pos;
}

static inline Vector2 get_cell_frac(const Entity* entity) {
    Vector2 pos = {
        fmodf(entity->pos.x, CELL_SIZE),
        fmodf(entity->pos.y, CELL_SIZE),
    };
    return pos;
}

void draw_ent(Entity* entity);
void draw_ent_debug(Entity* entity);
void ent_move(GameState* state, Entity* entity);
void ent_repel_ent(Entity* self, Entity* other);

// enumerate each occupied entry in an EntityArrayEntry - assumes that genarrays are always fully initialized to their max capacity
#define ENT_ARRAY_FOREACH(arr, iter) for (struct EntityArrayEntry* iter = &(arr).data[0]; iter != &(arr).data[MAX_ENTITIES-1]; iter++) if (iter->occupied)
#define ENT_REF_ARRAY_FOREACH(arr, iter) for (struct EntityArrayEntry* iter = &(arr)->data[0]; iter != &(arr)->data[MAX_ENTITIES-1]; iter++) if (iter->occupied)
