
#include <stdbool.h>

#include "entity.h"
#include "game.h"

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

int main(void) {
    SetTraceLogLevel(LOG_DEBUG);

    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "bwuh");

    // just in case vsync doesnt work, we'll set the target FPS to the current monitor's refresh rate + 1
    SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()) + 1);

    Assets game_assets = {
        .sheet = LoadTexture("assets/sheet.png"),
        .sheet_wall = {0, 8, 8, 8},
        .sheet_floor = {8, 8, 8, 8},
        .sheet_player = {72, 0, 16, 16},
    };

    const int render_width = 128, render_height = 128;

    GameData game_data = {
        .assets = &game_assets,
        .render_target = LoadRenderTexture(render_width, render_height),
        .debug = true,
        .dt = 1.0 / 50.0,
    };

    double current_time = GetTime();
    double accumulator = 0.0;

    GameState* previous_state = MemAlloc(sizeof(GameState));
    GameState* current_state = MemAlloc(sizeof(GameState));
    *current_state = (GameState){0};
    current_state->game_data = &game_data;
    current_state->player.cpos = (Vector2){ 3, 3 };
    *previous_state = *current_state;

    // assuming 16:9 aspect for now, calculate the integer scaling for our render texture
    int scale = (int)(GetRenderHeight() / (float)render_height);
    Rectangle target_dest = { 0, 0, render_width * scale, render_height * scale };
    // center render target
    target_dest.x = (GetRenderWidth() - target_dest.width) / 2;
    target_dest.y = (GetRenderHeight() - target_dest.height) / 2;

    // gafferongames fixed timestep
    while (!WindowShouldClose())
    {
        double new_time = GetTime();
        double frame_time = new_time - current_time;

        add_rolling_frame_time(frame_time);

        if (frame_time > 0.25)
            frame_time = 0.25;
        current_time = new_time;

        accumulator += frame_time;
        while (accumulator >= game_data.dt) {
            // TODO: get all input up front and set input state in GameData
            *previous_state = *current_state;
            integrate_state(current_state);
            game_data.t += game_data.dt;
            accumulator -= game_data.dt;
        }

        const double alpha = accumulator / game_data.dt;
        interpolate_state(previous_state, current_state, alpha);

        // target drawing
        BeginTextureMode(game_data.render_target);
        render_state(previous_state);
        EndTextureMode();

        // screen drawing
        BeginDrawing();

        ClearBackground(BLACK);
        DrawTexturePro(
            game_data.render_target.texture,
            // negative render height because funny opengl render target coordinates
            (Rectangle){ 0, 0, render_width, -render_height },
            target_dest,
            (Vector2){0, 0},
            0.0f,
            WHITE);

        EndDrawing();
    }

    UnloadTexture(game_assets.sheet);

    CloseWindow();
    return 0;
}