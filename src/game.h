#pragma once

#include "entity.h"
#include "input.h"

typedef struct Assets {
    Texture2D sheet;
    Sprite sprite_wall;
    Sprite sprite_floor;
    Sprite sprite_player;
    Sprite sprite_box;
} Assets;

typedef struct GameState GameState;
typedef struct GameData GameData;

typedef struct InputState {
    VirtualButton pause;

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
    InputState* input_state;

    bool paused;

    Entity player;
    EntityArray boxes;

    bool previously_moving_diag;
};

void update_input_state(GameData* state);
void integrate_state(GameState* state);
void interpolate_state(GameState* previous, GameState* current, double alpha);
void render_state(GameState* state);
void render_state_menu(GameState* state);

void draw_room(const GameState* state);
