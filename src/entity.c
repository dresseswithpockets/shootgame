#include <assert.h>
#include "game.h"
#include "genarray.h"
#include "sprite.h"
#include "entity.h"

GENARRAY_IMPL(Entity, MAX_ENTITIES, ent)

// get the world-position of the entity's radial center, in pixel coordinates
Vector2 get_world_radial_center(const Entity* entity) {
    return Vector2Add(entity->pos, entity->c_radial_center);
}

void draw_ent(Entity* entity) {
    // TODO: entities can have multiple sprites parented to them, with an arbitrary offset
    // for now, entities just have the one sprite, and it has no offset
    draw_sprite(entity->sprite, entity->pos, 0.0, WHITE);
}

void draw_ent_debug(Entity* entity) {
    DrawCircleV(entity->pos, 1.0, RED);
    // aabb rect
    Rectangle aabb = {
        entity->pos.x - entity->c_size.x / 2.0,
        entity->pos.y - entity->c_size.y / 2.0,
        entity->c_size.x,
        entity->c_size.y,
    };
    DrawRectangleLinesEx(aabb, 1.0f, YELLOW);
    // Rectangle aabb = entity->c_aabb;
    // aabb.x += pixel_pos.x - aabb.width / 2.0;
    // aabb.y += pixel_pos.y - aabb.height / 2.0;
    // DrawRectangleLinesEx(aabb, 1.0, YELLOW);
    // radius
    //Vector2 radial_center = Vector2Add(Vector2Scale(entity->c_radial_center, CELL_SIZE), pixel_pos);
    DrawCircleLinesV(get_world_radial_center(entity), entity->c_radius, BLUE);
}

bool tile_has_collision(int x, int y) {
    // rooms are always 16x16 with 1-unit-thick walls, so collision is trivial until
    // we add tiles that have collision
    return x == 0 || x == 15 || y == 0 || y == 15;
}

bool pixel_has_collision(int x, int y) {
    return x <= 7 || x >= 120 || y <= 7 || y >= 120;
}

void ent_repel_ent(Entity* self, Entity* other) {
    if (!self->c_pushes || !other->c_pushes) return;

    // fast AABB distance check, make sure theyre near eachother
    float total_radius = self->c_radius + other->c_radius;
    if (abs(self->pos.x - other->pos.x) <= total_radius && abs(self->pos.y - other->pos.y) <= total_radius) {
        // delta in cells
        Vector2 delta = Vector2Subtract(get_world_radial_center(other), get_world_radial_center(self));
        float distance = Vector2Length(delta);
        // radius in cells
        if (distance <= total_radius) {
            float angle = atan2f(delta.y, delta.x);
            float force = 4.0;
            float repel_power = (total_radius - distance) / total_radius;
            float dx = cosf(angle) * repel_power * force * (float)CELL_SIZE;
            float dy = sinf(angle) * repel_power * force * (float)CELL_SIZE;
            self->velocity.x -= dx;
            self->velocity.y -= dy;
            other->velocity.x += dx;
            other->velocity.y += dy;
        }
    }
}

void ent_move(GameState* state, Entity* entity) {
    // horizontal movement
    entity->pos_subpixel = Vector2Add(entity->pos_subpixel, Vector2Scale(entity->velocity, state->game_data->dt));
    entity->velocity = Vector2Scale(entity->velocity, entity->normal_friction);

    while (entity->pos_subpixel.x >= 1) {
        if (pixel_has_collision(entity->pos_pixel.x + (entity->c_size.x / 2) + 1, entity->pos_pixel.y)) {
            entity->pos_subpixel.x = 0.99;
            entity->velocity.x = 0;
        } else {
            entity->pos_subpixel.x -= 1;
            entity->pos_pixel.x += 1;
        }
    }
    while (entity->pos_subpixel.x < 0) {
        if (pixel_has_collision(entity->pos_pixel.x - (entity->c_size.x / 2) - 1, entity->pos_pixel.y)) {
            entity->pos_subpixel.x = 0;
            entity->velocity.x = 0;
        } else {
            entity->pos_subpixel.x += 1;
            entity->pos_pixel.x -= 1;
        }
    }

    while (entity->pos_subpixel.y >= 1) {
        if (pixel_has_collision(entity->pos_pixel.x, entity->pos_pixel.y + (entity->c_size.y / 2) + 1)) {
            entity->pos_subpixel.y = 0.99;
            entity->velocity.y = 0;
        } else {
            entity->pos_subpixel.y -= 1;
            entity->pos_pixel.y += 1;
        }
    }
    while (entity->pos_subpixel.y < 0) {
        if (pixel_has_collision(entity->pos_pixel.x, entity->pos_pixel.y - (entity->c_size.y / 2) - 1)) {
            entity->pos_subpixel.y = 0;
            entity->velocity.y = 0;
        } else {
            entity->pos_subpixel.y += 1;
            entity->pos_pixel.y -= 1;
        }
    }

    entity->pos = Vector2Add(v2itof(entity->pos_pixel), entity->pos_subpixel);

    // int horizontal_cell_size = (int)(entity->c_aabb.width / 2.0 / CELL_SIZE);
    // float horizontal_cell_remainder = 1.0 - fmodf(entity->c_aabb.width / 2.0, (float)CELL_SIZE);

    // if (tile_has_collision(entity->cpos.x + entity->c_size.x, entity->cpos.y) && entity->fpos.x >= 0.8) {
    //     entity->fpos.x = 0.8;
    //     entity->velocity.x = 0.0;
    // }
    // if (tile_has_collision(entity->cpos.x - 1, entity->cpos.y) && entity->fpos.x <= 0.3) {
    //     entity->fpos.x = 0.3;
    //     entity->velocity.x = 0.0;
    // }

    // while (entity->fpos.x > 1) {
    //     entity->cpos.x += 1;
    //     entity->fpos.x -= 1;
    // }
    // while (entity->fpos.x < 0) {
    //     entity->cpos.x -= 1;
    //     entity->fpos.x += 1;
    // }

    // // vertical movement
    // entity->fpos.y += entity->velocity.y * state->game_data->dt;
    // entity->velocity.y *= entity->normal_friction;

    // // HACK: tile collision here is kinda funky - it assumes that the player is 1x1 cells big
    // //       I have a suspicion it might be better to just store the subpixel position of the player
    // //       and to map it to tile coords when needed
    // if (tile_has_collision(entity->cpos.x, entity->cpos.y + entity->c_size.y) && entity->fpos.y >= 0.7) {
    //     entity->fpos.y = 0.7;
    //     entity->velocity.y = 0.0;
    // }
    // if (tile_has_collision(entity->cpos.x, entity->cpos.y - 1) && entity->fpos.y <= 0.3) {
    //     entity->fpos.y = 0.3;
    //     entity->velocity.y = 0.0;
    // }

    // while (entity->fpos.y > 1) {
    //     entity->cpos.y += 1;
    //     entity->fpos.y -= 1;
    // }
    // while (entity->fpos.y < 0) {
    //     entity->cpos.y -= 1;
    //     entity->fpos.y += 1;
    // }
}
