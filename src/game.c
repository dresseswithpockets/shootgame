#include "entity.h"
#include "game.h"

void integrate_state(GameState* state) {
    int in_x = 0, in_y = 0;
    if (IsKeyDown(KEY_A))
        in_x -= 1;
    if (IsKeyDown(KEY_D))
        in_x += 1;
    if (IsKeyDown(KEY_W))
        in_y -= 1;
    if (IsKeyDown(KEY_S))
        in_y += 1;

    if (in_x != 0 || in_y != 0) {
        float mag = sqrtf(in_x * in_x + in_y * in_y);
        state->player.velocity.x = 8 * in_x/mag;
        state->player.velocity.y = 8 * in_y/mag;
    } else {
        state->player.velocity.x = 0;
        state->player.velocity.y = 0;
    }

    // fix cobblestoning
    if (!state->previously_moving_diag && in_x != 0 && in_y != 0) {
        Vector2 pixel_pos = get_pixel_pos(&state->player);
        pixel_pos.x = floorf(pixel_pos.x) + 0.5;
        pixel_pos.y = floorf(pixel_pos.y) + 0.5;
        set_pixel_pos(&state->player, pixel_pos);
    }

    state->previously_moving_diag = in_x != 0 && in_y != 0;

    ent_move(state, &state->player);
}

void interpolate_entity(Entity* entity, Entity* next, double alpha) {
    // we will interpolate the entity's position, but we'll copy everything over from current
    // to previous, to ensure we're otherwise rendering the most correct information to the
    // player, as early as possible
    Vector2 previous_pos = get_pixel_pos(entity);

    // N.B. this assumes that any pointers in Entity are shared, and produce idempodent integrations
    *entity = *next;

    // for our interpolated/rendered state, only thing we care about for now is interpolating
    // positions
    set_pixel_pos(entity, Vector2Lerp(previous_pos, get_pixel_pos(next), alpha));
}

void interpolate_state(GameState* previous, GameState* current, double alpha) {
    interpolate_entity(&previous->player, &current->player, alpha);
}

void render_state(GameState* state) {
    ClearBackground(BLACK);
    draw_room(state);
    draw_player(state);
}

void draw_sheet_sprite(Texture2D sheet, Rectangle source, Vector2 pos) {
    DrawTexturePro(
        sheet,
        source,
        (Rectangle){ pos.x, pos.y, source.width, source.height },
        (Vector2){ source.width / 2.0f, source.height / 2.0f },
        0.0f,
        WHITE);
}

void draw_room(const GameState* state) {
    const GameData* game_data = state->game_data;
    for (int i = 0; i < 16; i++) {
        // top wall
        draw_sheet_sprite(
            game_data->assets->sheet,
            game_data->assets->sheet_wall,
            (Vector2){ i*8+4, 4 });

        // bottom wall
        draw_sheet_sprite(
            game_data->assets->sheet,
            game_data->assets->sheet_wall,
            (Vector2){ i*8+4, 124 });
    }

    for (int i = 1; i < 15; i++) {
        // left wall
        draw_sheet_sprite(
            game_data->assets->sheet,
            game_data->assets->sheet_wall,
            (Vector2){ 4, i*8+4 });

        // right wall
        draw_sheet_sprite(
            game_data->assets->sheet,
            game_data->assets->sheet_wall,
            (Vector2){ 124, i*8+4 });
    }
}