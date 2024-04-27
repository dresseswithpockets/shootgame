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

void integrate_player(GameState* state, Entity* player) {
    Vector2 wish_dir = (Vector2){ state->input_state->move_horizontal.value, state->input_state->move_vertical.value };
    if (wish_dir.x != 0 || wish_dir.y != 0) {
        wish_dir = Vector2Normalize(wish_dir);
    }

    Vector2 shoot_dir = (Vector2){ state->input_state->shoot_horizontal.value, state->input_state->shoot_vertical.value };
    if (shoot_dir.x != 0 || wish_dir.y != 0) {
        shoot_dir = Vector2Normalize(shoot_dir);
    }

    // NOTE(snale): honestly im not sure if this approach feels good, and using accelleration and
    //              friction might not be good for the bullet hell parts of the game. guess we'll
    //              see from playtesting.
    Vector2 wish_accel = Vector2Scale(wish_dir, state->game_data->dt * player->normal_max_speed / player->normal_accel_time);
    player->velocity = Vector2Add(player->velocity, wish_accel);
    float max_speed_sqr = player->normal_max_speed * player->normal_max_speed;
    if (Vector2LengthSqr(player->velocity) >= max_speed_sqr)
        player->velocity = Vector2Scale(Vector2Normalize(player->velocity), player->normal_max_speed);

    // fix cobblestoning
    // TODO: this still seems broken - there is a stutter, though it might be caused by friction?
    //       it might actually be because we aren't storing a subpixel position, only a subcell position
    //       that we truncate
    if (wish_dir.x != 0 && wish_dir.y != 0) {
        player->pos_subpixel.x = 0.5;
        player->pos_subpixel.y = 0.5;
        player->pos.x = floorf(player->pos.x) + 0.5;
        player->pos.y = floorf(player->pos.y) + 0.5;
    }

    state->previously_moving_diag = wish_dir.x != 0 && wish_dir.y != 0;

    // updating weapon source angle & positions
    if (shoot_dir.x != 0 || shoot_dir.y != 0) {
        //player->source_angle = move_towards(player->source_angle, atan2f(shoot_dir.y, shoot_dir.x), 2 * PI * state->game_data->dt);
        player->source_angle = smoothdamp_angle(
            player->source_angle,
            atan2f(shoot_dir.y, shoot_dir.x),
            10.0f,
            (float)state->game_data->dt);
    }

    for (int i = 0; i < ARRAY_LEN(player->sources); i++) {
        struct WeaponSource* source = &player->sources[i];
        float source_angle = player->source_angle + source->offset;
        source->pos = vector2_polar(10.0, source_angle);
    }

    ent_move(state, player);

    // ent_move sets up collision flags on the entity, check for player entering doors
    // TODO: very naive/broken room movement. Should set the player's position, and also change entity group
    if (HAS_COLLISION(player->c_flags)) {
        if (abs(player->pos.x - 64) < 4 && state->input_state->move_vertical.value != 0.0) {
            if (HAS_FLAG(player->c_flags, CollisionFlagUp) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionUp)) {
                state->room_idx.y -= 1;
                player->pos_pixel = (Vector2i) { 64, 111 };
            } else if (HAS_FLAG(player->c_flags, CollisionFlagDown) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionDown)) {
                state->room_idx.y += 1;
                player->pos_pixel = (Vector2i) { 64, 17 };
            }
        } else if (abs(player->pos.y - 64) < 4 && state->input_state->move_horizontal.value != 0.0) {
            if (HAS_FLAG(player->c_flags, CollisionFlagLeft) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionLeft)) {
                state->room_idx.x -= 1;
                player->pos_pixel = (Vector2i) { 111, 64 };
            } else if (HAS_FLAG(player->c_flags, CollisionFlagRight) && floor_has_room_dir(&state->floor_plan, state->room_idx, DirectionRight)) {
                state->room_idx.x += 1;
                player->pos_pixel = (Vector2i) { 17, 64 };
            }
        }

        player->pos = Vector2Add(v2itof(player->pos_pixel), player->pos_subpixel);
        player->room_idx = state->room_idx;
    }
}

void integrate_state(GameState* state) {
    if (state->input_state->pause.is_down && state->input_state->pause.this_frame) {
        state->paused = !state->paused;
    }

    if (state->paused) return;

    // example entity pushing
    ENT_ARRAY_FOREACH_INROOM(state->entities, entry_a, state->room_idx) {
        Entity* entity = &entry_a->value;
        ENT_ARRAY_FOREACH_INROOM(state->entities, entry_b, state->room_idx) {
            if (entry_a == entry_b) continue;
            ent_repel_ent(entity, &entry_b->value);
        }
    }

    ENT_ARRAY_FOREACH_INROOM(state->entities, entry, state->room_idx) {
        if (HAS_FLAG(entry->value.kind_flags, KindPlayer)) {
            integrate_player(state, &entry->value);
        } else if (HAS_FLAG(entry->value.kind_flags, KindBox)) {
            ent_move(state, &entry->value);
        }
    }
}

void interpolate_entity(Entity* entity, const Entity* next, double alpha) {
    // if the entity has changed rooms, it wouldnt make sense for it to interpolate from the old room,
    // so we want to just use the new positions we copied from the new entity state, rather than
    // overriding them. If we didnt do this, and just continued on to overriding positions,
    // interpolation would result in rendering the entities are the wrong position for a few frames
    if (!vector2i_eq(entity->room_idx, next->room_idx)) {
        *entity = *next;
        return;
    }

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
    ENT_ARRAY_FOREACH_INROOM(current->entities, entry, current->room_idx) {
        Entity* current_ent = &entry->value;
        Entity* previous_ent = ent_array_get(&previous->entities, current_ent->handle);
        if (previous_ent) {
            interpolate_entity(previous_ent, current_ent, alpha);
        }
    }

    // only the current state will have the paused flag if it was just paused this frame,
    // render_state only gets the interpolated previous state
    previous->paused = current->paused;
    previous->room_idx = current->room_idx;
}

void render_state(GameState* state) {
    ClearBackground(BLACK);
    draw_room(state);
    ENT_ARRAY_FOREACH_INROOM(state->entities, entry, state->room_idx) {
        draw_ent(&entry->value);
        if (state->game_data->debug) {
            draw_ent_debug(&entry->value);
        }
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

    if (state->game_data->debug) {
        if (vector2i_eq(state->room_idx, state->floor_plan.boss)) {
            DrawText("BOSS ROOM", 8, 8, 48, WHITE);
        }
        if (vector2i_eq(state->room_idx, state->floor_plan.item)) {
            DrawText("ITEM ROOM", 8, 8, 48, WHITE);
        }
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