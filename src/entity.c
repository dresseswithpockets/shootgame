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

    // radius
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
}
