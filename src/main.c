#include "entity.h"
#include "game.h"
#include "math.h"
#include "input.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

const int render_width = 128, render_height = 128;

void integrate_render_frame(void* game_data);

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
    SetConfigFlags(FLAG_VSYNC_HINT);
#endif

    InitWindow(1280, 720, "bwuh");

    Assets game_assets = {
        .sheet = LoadTexture("assets/sheet.png"),
        .sheet_wall = {0, 8, 8, 8},
        .sheet_floor = {8, 8, 8, 8},
        .sheet_player = {72, 0, 16, 16},
    };

    // assuming 16:9 aspect for now, calculate the integer scaling for our render texture
    int scale = (int)(GetRenderHeight() / (float)render_height);
    GameData game_data = {
        .assets = &game_assets,
        .render_target = LoadRenderTexture(render_width, render_height),
        .target_dest = { 0, 0, render_width * scale, render_height * scale },

        .previous_state = MemAlloc(sizeof(GameState)),
        .current_state = MemAlloc(sizeof(GameState)),

        .debug = true,
        .t = 0,
        .dt = 1.0 / 50.0,
        .current_time = GetTime(),
    };

    game_data.input_state = (InputState) {0};

    // setup default inputs
    virtual_axis_add_keys(&game_data.input_state.move_horizontal, KEY_A, KEY_D);
    virtual_axis_add_keys(&game_data.input_state.move_vertical, KEY_W, KEY_S);
    virtual_axis_add_keys(&game_data.input_state.shoot_horizontal, KEY_LEFT, KEY_RIGHT);
    virtual_axis_add_keys(&game_data.input_state.shoot_vertical, KEY_UP, KEY_DOWN);

    // center the render texture on the screen
    game_data.target_dest.x = (GetRenderWidth() - game_data.target_dest.width) / 2;
    game_data.target_dest.y = (GetRenderHeight() - game_data.target_dest.height) / 2;

    // setup some initial test data
    *game_data.current_state = (GameState){0};
    game_data.current_state->game_data = &game_data;
    game_data.current_state->player = (Entity) {
        .cpos = (Vector2i){ 3, 3 },
        .normal_friction = 0.85,
        .normal_accel_time = 0.02, // 1 ticks
        .normal_max_speed = 8.0,

        .c_radius = 6,
        .c_pushes = true,
    };
    for (int i = 0; i < ARRAY_LEN(game_data.current_state->boxes); i++) {
        game_data.current_state->boxes[i] = (Entity) {
            .cpos = (Vector2i) { GetRandomValue(5, 14), GetRandomValue(5, 14) },
            .fpos = (Vector2) { rand_float(0.0, 1.0), rand_float(0.0, 1.0) },
            .normal_friction = 0.96,

            .c_radius = 4,
            .c_pushes = true,
        };
    }
    *game_data.previous_state = *game_data.current_state;

#if defined(PLATFORM_WEB)
    // using -s ASYNCIFY instead of emscripten_set_main_loop_arg causes the update loop to grind to
    // a hault on firefox, so im doing it the proper way with a callback. Thankfully, with a fixed
    // timestemp, this should produce identical gameplay as the "real" loop.
    emscripten_set_main_loop_arg(integrate_render_frame, &game_data, 0, 1);
#else
    // just in case vsync doesnt work, we'll set the target FPS to the current monitor's refresh rate + 1
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));

    while (!WindowShouldClose())
        integrate_render_frame(&game_data);
#endif

    UnloadTexture(game_assets.sheet);

    CloseWindow();
    return 0;
}

void integrate_render_frame(void* user_data) {
    GameData* game_data = user_data;
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
        (Rectangle){ 0, 0, render_width, -render_height },
        game_data->target_dest,
        (Vector2){0, 0},
        0.0f,
        WHITE);

    EndDrawing();
}