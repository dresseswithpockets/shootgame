#include "entity.h"
#include "game.h"
#include "input.h"
#include "math.h"

void update_input_state(GameData* game_data) {
    virtual_axis_update(&game_data->input_state.move_horizontal);
    virtual_axis_update(&game_data->input_state.move_vertical);
    virtual_axis_update(&game_data->input_state.shoot_horizontal);
    virtual_axis_update(&game_data->input_state.shoot_vertical);
}

void integrate_player(GameState* state) {
    const GameData* game_data = state->game_data;

    int in_x = signi(game_data->input_state.move_horizontal.value);
    int in_y = signi(game_data->input_state.move_vertical.value);

    Vector2 wish_dir = (Vector2){ 0.0, 0.0 };
    if (in_x != 0 || in_y != 0) {
        float mag = sqrtf(in_x * in_x + in_y * in_y);
        wish_dir = (Vector2){ in_x / mag, in_y / mag };
    }

    // NOTE(snale): honestly im not sure if this approach feels good, and using accelleration and
    //              friction might not be good for the bullet hell parts of the game. guess we'll
    //              see from playtesting.
    Vector2 wish_accel = Vector2Scale(wish_dir, state->game_data->dt * state->player.normal_max_speed / state->player.normal_accel_time);
    state->player.velocity = Vector2Add(state->player.velocity, wish_accel);
    float max_speed_sqr = state->player.normal_max_speed * state->player.normal_max_speed;
    if (Vector2LengthSqr(state->player.velocity) >= max_speed_sqr)
        state->player.velocity = Vector2Scale(Vector2Normalize(state->player.velocity), state->player.normal_max_speed);

    // fix cobblestoning
    // TODO: this still seems broken - there is a stutter, though it might be caused by friction?
    //       it might actually be because we aren't storing a subpixel position, only a subcell position
    //       that we truncate
    if (!state->previously_moving_diag && in_x != 0 && in_y != 0) {
        Vector2 pixel_pos = get_pixel_pos(&state->player);
        pixel_pos.x = floorf(pixel_pos.x) + 0.5;
        pixel_pos.y = floorf(pixel_pos.y) + 0.5;
        set_pixel_pos(&state->player, pixel_pos);
    }

    state->previously_moving_diag = in_x != 0 && in_y != 0;
}

void integrate_state(GameState* state) {
    integrate_player(state);

    // example entity pushing
    ENT_ARRAY_FOREACH(state->boxes, box_entry_a) {
        Entity* box = &box_entry_a->value;
        ent_repel_ent(&state->player, box);

        ENT_ARRAY_FOREACH(state->boxes, box_entry_b) {
            if (box_entry_a == box_entry_b) continue;
            ent_repel_ent(box, &box_entry_b->value);
        }
    }

    ENT_ARRAY_FOREACH(state->boxes, box_entry) {
        ent_move(state, &box_entry->value);
    }

    ent_move(state, &state->player);
}

void interpolate_entity(Entity* entity, const Entity* next, double alpha) {
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

    ENT_ARRAY_FOREACH(current->boxes, box_entry) {
        Entity* current_ent = &box_entry->value;
        Entity* previous_ent = ent_array_get(&previous->boxes, current_ent->handle);
        if (previous_ent) {
            interpolate_entity(previous_ent, current_ent, alpha);
        }
    }
}

void render_state(GameState* state) {
    ClearBackground(BLACK);
    draw_room(state);
    draw_player(state);
    draw_box(state);
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