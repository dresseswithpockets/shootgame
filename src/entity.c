#include <stdio.h>

#include "game.h"
#include "entity.h"

void draw_player(GameState* state) {
    Assets* game_assets = state->game_data->assets;
    Entity* player = &state->player;
    Vector2 pos = get_pixel_pos(player);
    draw_sheet_sprite(game_assets->sheet, game_assets->sheet_player, pos);
}

void ent_move(GameState* state, Entity* entity) {
    entity->fpos.x += entity->velocity.x * state->game_data->dt;
    // TODO: tile collision
    while (entity->fpos.x > 1) {
        entity->cpos.x += 1;
        entity->fpos.x -= 1;
    }
    while (entity->fpos.x < 0) {
        entity->cpos.x -= 1;
        entity->fpos.x += 1;
    }

    entity->fpos.y += entity->velocity.y * state->game_data->dt;
    // TODO: tile collision
    while (entity->fpos.y > 1) {
        entity->cpos.y += 1;
        entity->fpos.y -= 1;
    }
    while (entity->fpos.y < 0) {
        entity->cpos.y -= 1;
        entity->fpos.y += 1;
    }
}
