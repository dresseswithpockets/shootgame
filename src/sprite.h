#pragma once

typedef struct Sprite {
    Texture2D sprite_sheet;
    Rectangle source;
    Vector2 origin;
} Sprite;

void draw_sprite(Sprite* sprite, Vector2 position);
void draw_sprite_ex(Sprite* sprite, Vector2 position, float rotation, Color modulate, bool flipx, bool flipy);
