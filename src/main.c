#include "dungeon.h"
#include "entity.h"
#include "game.h"
#include "math.h"
#include "input.h"
#include "sprite.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// libc forward decl
void* __cdecl memset(void* dest, int val, size_t size);

const int render_width = 128, render_height = 128;

void integrate_render_frame(void *user_data);

void add_rolling_frame_time(double frame_time) {
    static double rolling_frame_time[100];
    static int frame_idx = 0;

    // rolling framerate average for a more useful number
    rolling_frame_time[frame_idx] = frame_time;
    frame_idx = (frame_idx + 1) % 100;
    double rolling_average = 0;
    for (int i = 0; i < 100; i++) rolling_average += rolling_frame_time[i];
    rolling_average = 100.0 / rolling_average;
    SetWindowTitle(TextFormat("fps: %lf", rolling_average));
}

// gross, i know, but it works and this is the only place im gunna be doing this funny interweaved platform ifdef stuff
#ifdef SG_USE_WINMAIN
int WinMain(void* hInstance, void* hPrevInstance, char* lpCmdLine, int nCmdShow) {
#else

int main(void) {
#endif
    SetTraceLogLevel(LOG_DEBUG);

// setting VSYNC on desktop/non-web platforms
#if !defined(PLATFORM_WEB)
    //SetConfigFlags(FLAG_VSYNC_HINT);
#endif

    InitWindow(1280, 720, "bwuh");
    SetExitKey(KEY_F1);

    Texture2D sprite_sheet = LoadTexture("assets/sheet.png");
    Assets game_assets = {
        .sheet = sprite_sheet,
        .sprite_wall = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = {0, 8, 8, 8},
            .origin = {0, 0},
        },
        .sprite_floor = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = {8, 8, 8, 8},
            .origin = {0, 0},
        },
        .sprite_player_down = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = {72, 0, 16, 16},
            .origin = {8, 8},
        },
        .sprite_player_left = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = {88, 0, 16, 16},
            .origin = {8, 8},
        },
        .sprite_player_up = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = {104, 0, 16, 16},
            .origin = {8, 8},
        },
        .sprite_box = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = {8, 0, 8, 8},
            .origin = {4, 4},
        },
        .sprite_door_normal = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = {24, 8, 16, 8},
            .origin = {8, 4},
        },
        .sprite_source = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = { 120, 16, 5, 5 },
            .origin = { 2.6, 2.5 },
        },
        .sprite_bullet_up = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = { 1, 24, 6, 8 },
            .origin = { 2.5, 2.5 },
        },
        .sprite_bullet_left = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = { 8, 24, 8, 6 },
            .origin = { 2.5, 2.5 },
        },
        .sprite_bullet_diag = (Sprite) {
            .sprite_sheet = sprite_sheet,
            .source = { 16, 24, 8, 8 },
            .origin = { 3.5, 3.5 },
        },
    };

    // assuming 16:9 aspect for now, calculate the integer scaling for our render texture
    int scale = (int) (GetRenderHeight() / (float) render_height);
    GameData game_data = {
        .assets = &game_assets,
        .render_target = LoadRenderTexture(render_width, render_height),
        .target_dest = {0, 0, render_width * scale, render_height * scale},

        .previous_state = MemAlloc(sizeof(GameState)),
        .current_state = MemAlloc(sizeof(GameState)),

        .debug = false,
        .t = 0,
        .dt = 1.0 / 50.0,
        .current_time = GetTime(),
    };

    game_data.input_state = (InputState) {0};

    // setup default inputs
    virtual_button_add_key(&game_data.input_state.pause, KEY_ESCAPE);
    virtual_axis_add_keys(&game_data.input_state.move_horizontal, KEY_A, KEY_D);
    virtual_axis_add_keys(&game_data.input_state.move_vertical, KEY_W, KEY_S);
    virtual_axis_add_keys(&game_data.input_state.shoot_horizontal, KEY_LEFT, KEY_RIGHT);
    virtual_axis_add_keys(&game_data.input_state.shoot_vertical, KEY_UP, KEY_DOWN);

    // center the render texture on the screen
    game_data.target_dest.x = (GetRenderWidth() - game_data.target_dest.width) / 2;
    game_data.target_dest.y = (GetRenderHeight() - game_data.target_dest.height) / 2;

    game_data.current_state->game_data = &game_data;
    game_data.current_state->input_state = &game_data.input_state;
    game_data.current_state->room_idx = (Vector2i) {2, 2};

    ent_array_init(&game_data.current_state->entities);

    // setup player at center room
    {
        Handle new_player_handle = ent_array_insert_new(&game_data.current_state->entities);
        Entity* player = ent_array_get(&game_data.current_state->entities, new_player_handle);
        ent_init_player(player, game_data.assets, game_data.current_state->room_idx);
    }

    // setup test pushable boxes
    {
        for (int i = 0; i < 16; i++) {
            Handle new_box_handle = ent_array_insert_new(&game_data.current_state->entities);
            Entity* box = ent_array_get(&game_data.current_state->entities, new_box_handle);
            ent_init_box(box, game_data.assets, game_data.current_state->room_idx);
        }
    }

    // generate initial floor plan. If it fails, try again, until it succeeds
    while (!floor_plan_generate(&game_data.current_state->floor_plan, 1));

    // this method of deep copying requires that the game state hold no pointers with state lifetimes
    // e.g. InputState and GameData have lifetimes beyond the state's lifetime, and its expected
    // that those pointers get copied between the current and previous state, so its fine
    *game_data.previous_state = *game_data.current_state;

#if defined(PLATFORM_WEB)
    // using -s ASYNCIFY instead of emscripten_set_main_loop_arg causes the update loop to grind to
    // a hault on firefox, so im doing it the proper way with a callback. Thankfully, with a fixed
    // timestemp, this should produce identical gameplay as the "real" loop.
    emscripten_set_main_loop_arg(integrate_render_frame, &game_data, 0, 1);
#else
    // just in case vsync doesnt work, we'll set the target FPS to the current monitor's refresh rate + 1
    //SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()) + 1);
    SetTargetFPS(60);

    while (!WindowShouldClose())
        integrate_render_frame(&game_data);
#endif

    MemFree(game_data.previous_state);
    MemFree(game_data.current_state);
    UnloadTexture(game_assets.sheet);

    CloseWindow();
    return 0;
}

void integrate_render_frame(void *user_data) {
    GameData *game_data = user_data;
    // gafferongames fixed timestep
    static double frame_accumulator = 0.0;

    double new_time = GetTime();
    double frame_time = new_time - game_data->current_time;

    add_rolling_frame_time(frame_time);

    if (frame_time > 0.25)
        frame_time = 0.25;
    game_data->current_time = new_time;

    frame_accumulator += frame_time;
    while (frame_accumulator >= game_data->dt) {
        update_input_state(game_data);
        // TODO: get all input up front and set input state in GameData
        *game_data->previous_state = *game_data->current_state;
        integrate_state(game_data->current_state);
        game_data->t += game_data->dt;
        frame_accumulator -= game_data->dt;
    }

    const double alpha = frame_accumulator / game_data->dt;
    interpolate_state(game_data->previous_state, game_data->current_state, alpha);

    // target drawing
    BeginTextureMode(game_data->render_target);
    render_state(game_data->previous_state);
    EndTextureMode();

    // screen drawing
    BeginDrawing();

    ClearBackground(BLACK);
    DrawTexturePro(
        game_data->render_target.texture,
        // negative render height because funny opengl render target coordinates
        (Rectangle) {0, 0, render_width, -render_height},
        game_data->target_dest,
        (Vector2) {0, 0},
        0.0f,
        WHITE);

    render_state_menu(game_data->previous_state);

    EndDrawing();
}