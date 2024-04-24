#include "game.h"
#include "entity.h"

void draw_player(GameState* state) {
    Assets* game_assets = state->game_data->assets;
    Entity* player = &state->player;
    Vector2 pos = get_pixel_pos(player);
    draw_sheet_sprite(game_assets->sheet, game_assets->sheet_player, pos);
}

void draw_box(GameState* state) {
    for (int i = 0; i < ARRAY_LEN(state->boxes); i++) {
        Assets* game_assets = state->game_data->assets;
        Entity* box = &state->boxes[i];
        Vector2 pos = get_pixel_pos(box);
        draw_sheet_sprite(game_assets->sheet, (Rectangle) { 8, 0, 8, 8 }, pos);
    }
}

bool tile_has_collision(int x, int y) {
    // rooms are always 16x16 with 1-unit-thick walls, so collision is trivial until
    // we add tiles that have collision
    return x == 0 || x == 15 || y == 0 || y == 15;
}

void ent_repel_ent(Entity* self, Entity* other) {
    if (!self->c_pushes || !other->c_pushes) return;

    // fast distance check, make sure theyre near eachother
    if (abs(self->cpos.x - other->cpos.x) <= 2 && abs(self->cpos.y - other->cpos.y) <= 2) {
        Vector2 delta = Vector2Subtract(get_pixel_pos(other), get_pixel_pos(self));
        float distance = Vector2Length(delta);
        float total_radius = self->c_radius + other->c_radius;
        if (distance <= total_radius) {
            float angle = atan2f(delta.y, delta.x);
            float force = 8.0;
            float repel_power = (total_radius - distance) / total_radius;
            float dx = cosf(angle) * repel_power * force;
            float dy = sinf(angle) * repel_power * force;
            self->velocity.x -= dx;
            self->velocity.y -= dy;
            other->velocity.x += dx;
            other->velocity.y += dy;
        }
    }
}

void ent_move(GameState* state, Entity* entity) {
    // horizontal movement
    entity->fpos.x += entity->velocity.x * state->game_data->dt;
    entity->velocity.x *= entity->normal_friction;

    // HACK: tile collision here is kinda funky - it assumes that the player is 1x1 cells big
    //       I have a suspicion it might be better to just store the subpixel position of the player
    //       and to map it to tile coords when needed
    if (tile_has_collision(entity->cpos.x + 1, entity->cpos.y) && entity->fpos.x >= 0.5) {
        entity->fpos.x = 0.5;
        entity->velocity.x = 0.0;
    }
    if (tile_has_collision(entity->cpos.x - 1, entity->cpos.y) && entity->fpos.x <= 0.5) {
        entity->fpos.x = 0.5;
        entity->velocity.x = 0.0;
    }

    while (entity->fpos.x > 1) {
        entity->cpos.x += 1;
        entity->fpos.x -= 1;
    }
    while (entity->fpos.x < 0) {
        entity->cpos.x -= 1;
        entity->fpos.x += 1;
    }

    // vertical movement
    entity->fpos.y += entity->velocity.y * state->game_data->dt;
    entity->velocity.y *= entity->normal_friction;

    // HACK: tile collision here is kinda funky - it assumes that the player is 1x1 cells big
    //       I have a suspicion it might be better to just store the subpixel position of the player
    //       and to map it to tile coords when needed
    if (tile_has_collision(entity->cpos.x, entity->cpos.y + 1) && entity->fpos.y >= 0.5) {
        entity->fpos.y = 0.5;
        entity->velocity.y = 0.0;
    }
    if (tile_has_collision(entity->cpos.x, entity->cpos.y - 1) && entity->fpos.y <= 0.5) {
        entity->fpos.y = 0.5;
        entity->velocity.y = 0.0;
    }

    while (entity->fpos.y > 1) {
        entity->cpos.y += 1;
        entity->fpos.y -= 1;
    }
    while (entity->fpos.y < 0) {
        entity->cpos.y -= 1;
        entity->fpos.y += 1;
    }
}
