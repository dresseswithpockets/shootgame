#pragma once

#include <raylib.h>
#include "entity.h"

typedef struct Assets {
    Texture2D sheet;
    Rectangle sheet_wall;
    Rectangle sheet_floor;
    Rectangle sheet_player;
} Assets;

typedef struct GameData {
    RenderTexture2D render_target;
    Assets* assets;

    bool debug;
    double t;
    double dt;
} GameData;

typedef struct GameState {
    GameData* game_data;

    Entity player;
    bool previously_moving_diag;
} GameState;

void integrate_state(GameState* state);
void interpolate_state(GameState* previous, GameState* current, double alpha);
void render_state(GameState* state);

void draw_sheet_sprite(Texture2D sheet, Rectangle source, Vector2 pos);
void draw_room(const GameState* state);
