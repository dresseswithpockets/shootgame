#include "game.h"
#include "entity.h"

void draw_player(GameState* state) {
    Assets* game_assets = state->game_data->assets;
    Entity* player = &state->player;
    Vector2 pos = get_pixel_pos(player);
    draw_sheet_sprite(game_assets->sheet, game_assets->sheet_player, pos);
}

bool tile_has_collision(int x, int y) {
    // rooms are always 16x16 with 1-unit-thick walls, so collision is trivial until
    // we add tiles that have collision
    return x == 0 || x == 15 || y == 0 || y == 15;
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
