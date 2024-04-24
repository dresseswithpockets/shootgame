#pragma once

#include "entity.h"
#include "input.h"

typedef struct Assets {
    Texture2D sheet;
    Rectangle sheet_wall;
    Rectangle sheet_floor;
    Rectangle sheet_player;
} Assets;

typedef struct GameState GameState;
typedef struct GameData GameData;

typedef struct InputState {
    VirtualAxis move_horizontal;
    VirtualAxis move_vertical;

    VirtualAxis shoot_horizontal;
    VirtualAxis shoot_vertical;
} InputState;

struct GameData {
    Assets* assets;
    RenderTexture2D render_target;
    Rectangle target_dest;

    InputState input_state;

    GameState* previous_state;
    GameState* current_state;

    bool debug;
    double t;
    double dt;
    double current_time;
};

struct GameState {
    GameData* game_data;

    Entity player;
    Entity boxes[64];

    bool previously_moving_diag;
};

void update_input_state(GameData* state);
void integrate_state(GameState* state);
void interpolate_state(GameState* previous, GameState* current, double alpha);
void render_state(GameState* state);

void draw_sheet_sprite(Texture2D sheet, Rectangle source, Vector2 pos);
void draw_room(const GameState* state);
