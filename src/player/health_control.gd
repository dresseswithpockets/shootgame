extends Control
class_name HealthControl

@export var texture: Texture2D
@export var region: Rect2

func _ready() -> void:
    # whenever the player is damage, redraw health
    Global.player_damaged.connect(queue_redraw)

func _draw() -> void:
    for i in range(Global.player.health):
        var rect := Rect2(i * (region.size.x + 1), 0, region.size.x, region.size.y)
        draw_texture_rect_region(texture, rect, region)
