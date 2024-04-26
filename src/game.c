#include "entity.h"
#include "game.h"
#include "input.h"
#include "math.h"

void update_input_state(GameData* game_data) {
    virtual_button_update(&game_data->input_state.pause);
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
    if (in_x != 0 && in_y != 0) {
        state->player.pos_subpixel.x = 0.5;
        state->player.pos_subpixel.y = 0.5;
        state->player.pos.x = floorf(state->player.pos.x) + 0.5;
        state->player.pos.y = floorf(state->player.pos.y) + 0.5;
    }

    state->previously_moving_diag = in_x != 0 && in_y != 0;

    ent_move(state, &state->player);

    // ent_move sets up collision flags on the entity, check for player entering doors
    // TODO: very naive/broken room movement. Should set the player's position, and also change entity group
    if (HAS_COLLISION(state->player.c_flags)) {
        if (abs(state->player.pos.x - 64) < 4 && state->input_state->move_vertical.value != 0.0) {
            if (HAS_FLAG(state->player.c_flags, CollisionFlagUp) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionUp)) {
                state->room_idx.y -= 1;
            } else if (HAS_FLAG(state->player.c_flags, CollisionFlagDown) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionDown)) {
                state->room_idx.y += 1;
            }
        } else if (abs(state->player.pos.y - 64) < 4 && state->input_state->move_horizontal.value != 0.0) {
            if (HAS_FLAG(state->player.c_flags, CollisionFlagLeft) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionLeft)) {
                state->room_idx.x -= 1;
            } else if (HAS_FLAG(state->player.c_flags, CollisionFlagRight) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionRight)) {
                state->room_idx.x += 1;
            }
        }
    }
}

void integrate_state(GameState* state) {
    if (state->input_state->pause.is_down && state->input_state->pause.this_frame) {
        state->paused = !state->paused;
    }

    if (state->paused) return;

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
}

void interpolate_entity(Entity* entity, const Entity* next, double alpha) {
    // we will interpolate the entity's position, but we'll copy everything over from current
    // to previous, to ensure we're otherwise rendering the most correct information to the
    // player, as early as possible
    Vector2i previous_pos_pixel = entity->pos_pixel;
    Vector2 previous_pos = entity->pos;

    // N.B. this assumes that any pointers in Entity are shared, and produce idempodent integrations
    *entity = *next;

    // for our interpolated/rendered state, only thing we care about for now is interpolating
    // positions
    entity->pos_pixel = vector2i_lerp(previous_pos_pixel, next->pos_pixel, alpha);
    entity->pos = Vector2Lerp(previous_pos, next->pos, alpha);
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

    // only the current state will have the paused flag if it was just paused this frame,
    // render_state only gets the interpolated previous state
    previous->paused = current->paused;
}

void render_state(GameState* state) {
    ClearBackground(BLACK);
    draw_room(state);

    ENT_ARRAY_FOREACH(state->boxes, box_entry) {
        draw_ent(&box_entry->value);
        if (state->game_data->debug) {
            draw_ent_debug(&box_entry->value);
        }
    }
    draw_ent(&state->player);
    if (state->game_data->debug) {
        draw_ent_debug(&state->player);
    }
}

void render_state_menu(GameState* state) {
    if (state->paused) {
        int half_width = GetRenderWidth() / 2;
        int third_height = GetRenderHeight() / 3;
        int measure = MeasureText("paused", 64) / 2;
        int border = 2;
        DrawText("paused", half_width - measure - border, third_height, 64, WHITE);
        DrawText("paused", half_width - measure + border, third_height, 64, WHITE);
        DrawText("paused", half_width - measure, third_height - border, 64, WHITE);
        DrawText("paused", half_width - measure, third_height + border, 64, WHITE);
        DrawText("paused", half_width - measure, third_height, 64, BLACK);
    }
}

void draw_room(const GameState* state) {
    const GameData* game_data = state->game_data;
    for (int i = 0; i < 16; i++) {
        // top wall
        draw_sprite(&game_data->assets->sprite_wall, (Vector2){ i*8, 0 });
        // bottom wall
        draw_sprite(&game_data->assets->sprite_wall, (Vector2){ i*8, 120 });
    }

    for (int i = 1; i < 15; i++) {
        // left wall
        draw_sprite(&game_data->assets->sprite_wall, (Vector2){ 0, i*8 });

        // right wall
        draw_sprite(&game_data->assets->sprite_wall, (Vector2){ 120, i*8 });
    }

    // top door
    if (state->room_idx.y > 0 && state->floor_plan.rooms[state->room_idx.x][state->room_idx.y - 1]) {
        draw_sprite_ex(&game_data->assets->sprite_door_normal, (Vector2){ 64, 4 }, 0.0, WHITE, false, false);
    }

    // bottom door
    if (state->room_idx.y < FLOOR_HEIGHT - 1 && state->floor_plan.rooms[state->room_idx.x][state->room_idx.y + 1]) {
        draw_sprite_ex(&game_data->assets->sprite_door_normal, (Vector2){ 64, 124 }, 0.0, WHITE, false, true);
    }

    // left door
    if (state->room_idx.x > 0 && state->floor_plan.rooms[state->room_idx.x - 1][state->room_idx.y]) {
        draw_sprite_ex(&game_data->assets->sprite_door_normal, (Vector2){ 4, 64 }, -90, WHITE, false, false);
    }

    // right door
    if (state->room_idx.x < FLOOR_WIDTH - 1 && state->floor_plan.rooms[state->room_idx.x + 1][state->room_idx.y]) {
        draw_sprite_ex(&game_data->assets->sprite_door_normal, (Vector2){ 124, 64 }, -90, WHITE, false, true);
    }
}