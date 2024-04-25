#include "sprite.h"

void draw_sprite(Sprite* sprite, Vector2 position, float rotation, Color modulate) {
    DrawTexturePro(
        sprite->sprite_sheet,
        sprite->source,
        (Rectangle) { position.x, position.y, sprite->source.width, sprite->source.height },
        sprite->origin,
        rotation,
        modulate
    );
}