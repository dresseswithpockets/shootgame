#pragma once

#include "entity.h"

typedef struct Assets {
    Texture2D sheet;
    Rectangle sheet_wall;
    Rectangle sheet_floor;
    Rectangle sheet_player;
} Assets;

typedef struct GameState GameState;
typedef struct GameData GameData;

typedef struct KeyState {
    // key to check
    KeyboardKey key;
    // number of frames that this key has been in its current state. starts at 0, meaning current frame
    int counter;
    // true if counter == 0; that is, if this is the first frame that is_down has been true or false.
    bool this_frame;
    // whether or not the key is currently pressed down
    bool is_down;
} KeyState;

typedef struct InputState {
    KeyState move_up;
    KeyState move_down;
    KeyState move_left;
    KeyState move_right;

    KeyState shoot_up;
    KeyState shoot_down;
    KeyState shoot_left;
    KeyState shoot_right;
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
