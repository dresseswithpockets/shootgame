#include "sprite.h"

void draw_sprite(Sprite* sprite, Vector2 position) {
    DrawTexturePro(
        sprite->sprite_sheet,
        sprite->source,
        (Rectangle) { position.x, position.y, sprite->source.width, sprite->source.height },
        sprite->origin,
        0.0,
        WHITE
    );
}

void draw_sprite_ex(Sprite* sprite, Vector2 position, float rotation, Color modulate, bool flipx, bool flipy) {
    Rectangle source = sprite->source;
    source.width *= (flipx ? -1 : 1);
    source.height *= (flipy ? -1 : 1);
    DrawTexturePro(
        sprite->sprite_sheet,
        source,
        (Rectangle) { position.x, position.y, sprite->source.width, sprite->source.height },
        sprite->origin,
        rotation,
        modulate
    );
}