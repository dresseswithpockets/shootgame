#pragma once

// forward decl needed for set_pixel_pos
float fmodf(float x, float y);

// game.h forward decls
typedef struct GameData GameData;
typedef struct GameState GameState;

#define CELL_SIZE 8

typedef struct Entity {
    // position on tile grid
    Vector2 cpos;
    // fraction from 0 to 1 representing sub-grid position
    Vector2 fpos;

    // velocity in cells per second
    Vector2 velocity;

    // AABB collision info used for collisions with tiles
    Rectangle c_aabb;
    // Radius collision info used for character pushing and bullet intersections
    Vector2 c_radial_center;
    float c_radius;

    int health;
    int max_health;
} Entity;

static inline Vector2 get_pixel_pos(Entity* entity) {
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
void ent_move(GameState* state, Entity* entity);
