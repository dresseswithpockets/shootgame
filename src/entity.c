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
    draw_sprite_ex(entity->sprite, entity->pos, entity->s_rotation, WHITE, entity->flip_x, entity->flip_y);

    // draw occupied sources/sources the entity actually has
    for (int i = 0; i < ARRAY_LEN(entity->sources); i++) {
        if (!entity->sources[i].occupied) continue;
        draw_sprite(entity->sources[i].sprite, Vector2Add(entity->pos, entity->sources->pos));
    }
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

void ent_bullet_test(Entity* bullet, Entity* other) {
    // TODO:
}

void ent_move(GameState* state, Entity* entity) {
    // horizontal movement
    entity->pos_subpixel = Vector2Add(entity->pos_subpixel, Vector2Scale(entity->velocity, state->game_data->dt));
    entity->velocity = Vector2Scale(entity->velocity, entity->normal_friction);

    entity->c_flags = 0;
    while (entity->pos_subpixel.x >= 1) {
        if (entity->c_collide_world && pixel_has_collision(entity->pos_pixel.x + (entity->c_size.x / 2) + 1, entity->pos_pixel.y)) {
            entity->pos_subpixel.x = 0.99;
            entity->velocity.x = 0;
            entity->c_flags |= CollisionFlagRight;
        } else {
            entity->pos_subpixel.x -= 1;
            entity->pos_pixel.x += 1;
        }
    }
    while (entity->pos_subpixel.x < 0) {
        if (entity->c_collide_world && pixel_has_collision(entity->pos_pixel.x - (entity->c_size.x / 2) - 1, entity->pos_pixel.y)) {
            entity->pos_subpixel.x = 0;
            entity->velocity.x = 0;
            entity->c_flags |= CollisionFlagLeft;
        } else {
            entity->pos_subpixel.x += 1;
            entity->pos_pixel.x -= 1;
        }
    }

    while (entity->pos_subpixel.y >= 1) {
        if (entity->c_collide_world && pixel_has_collision(entity->pos_pixel.x, entity->pos_pixel.y + (entity->c_size.y / 2) + 1)) {
            entity->pos_subpixel.y = 0.99;
            entity->velocity.y = 0;
            entity->c_flags |= CollisionFlagDown;
        } else {
            entity->pos_subpixel.y -= 1;
            entity->pos_pixel.y += 1;
        }
    }
    while (entity->pos_subpixel.y < 0) {
        if (entity->c_collide_world && pixel_has_collision(entity->pos_pixel.x, entity->pos_pixel.y - (entity->c_size.y / 2) - 1)) {
            entity->pos_subpixel.y = 0;
            entity->velocity.y = 0;
            entity->c_flags |= CollisionFlagUp;
        } else {
            entity->pos_subpixel.y += 1;
            entity->pos_pixel.y -= 1;
        }
    }

    entity->pos = Vector2Add(v2itof(entity->pos_pixel), entity->pos_subpixel);
}

void ent_init_player(Entity* entity, Assets* assets, Vector2i room_idx) {
    *entity = (Entity) {
        .handle = entity->handle,

        .kind_flags = KindPlayer,
        .room_idx = room_idx,

        .pos_pixel = (Vector2i) {24, 24},
        .pos = (Vector2) {24, 24},

        .normal_friction = 0.85,
        .normal_accel_time = 0.02, // 1 ticks
        .normal_max_speed = 64.0, // pixels per second

        .sprite = &assets->sprite_player_down,

        .c_collide_world = true,
        .c_size = {8, 14},
        .c_radius = 6,
        .c_pushes = true,
    };
    entity->sources[0] = (struct WeaponSource) {
        .occupied = true,
        .sprite = &assets->sprite_source,
        .delay = 0.5,
    };
}

void ent_init_box(Entity* entity, Assets* assets, Vector2i room_idx) {
    Vector2i pos_pixel = (Vector2i) {GetRandomValue(40, 112), GetRandomValue(40, 112)};
    *entity = (Entity) {
        .handle = entity->handle,

        .kind_flags = KindBox,
        .room_idx = room_idx,

        .pos_pixel = pos_pixel,
        .pos = v2itof(pos_pixel),
        .normal_friction = 0.96,

        .sprite = &assets->sprite_box,

        .c_collide_world = true,
        .c_size = {8, 8},
        .c_radius = 3,
        .c_pushes = true,
    };
}

void ent_init_bullet(Entity* entity, Assets* assets, Vector2i room_idx, Vector2 pos, Vector2 dir) {
    *entity = (Entity) {
        .handle = entity->handle,

        .kind_flags = KindBullet,
        .room_idx = room_idx,

        .pos_pixel = v2ftoi(pos),
        .pos_subpixel = vector2_subpixel(pos),
        .pos = pos,

        .velocity = Vector2Scale(dir, 64),
        .normal_friction = 1.0,

        .sprite = &assets->sprite_bullet_up,

        .c_collide_world = false,
        .c_size = { 2, 2 },
        .c_radius = 2,
        .c_pushes = false,
    };
}
