#include "entity.h"
#include "game.h"

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

#if !defined(PLATFORM_WEB)
    // just in case vsync doesnt work, we'll set the target FPS to the current monitor's refresh rate + 1
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
#endif

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

    // center the renter texture on the screen
    game_data.target_dest.x = (GetRenderWidth() - game_data.target_dest.width) / 2;
    game_data.target_dest.y = (GetRenderHeight() - game_data.target_dest.height) / 2;

    // setup some initial test data
    *game_data.current_state = (GameState){0};
    game_data.current_state->game_data = &game_data;
    game_data.current_state->player.cpos = (Vector2){ 3, 3 };
    *game_data.previous_state = *game_data.current_state;

#if defined(PLATFORM_WEB)
    // using -s ASYNCIFY instead of emscripten_set_main_loop_arg causes the update loop to grind to
    // a hault on firefox, so im doing it the proper way with a callback. Thankfully, with a fixed
    // timestemp, this should produce identical gameplay as the "real" loop.
    emscripten_set_main_loop_arg(integrate_render_frame, &game_data, 0, 1);
#else
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