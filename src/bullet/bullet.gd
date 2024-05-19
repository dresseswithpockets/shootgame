extends Node2D
class_name Bullet

enum { TEAM_PLAYER, TEAM_ENEMY }

@onready var area: Area2D = $Area2D
@onready var sprite_north: Sprite2D = $SpriteNorth
@onready var sprite_west: Sprite2D = $SpriteWest
@onready var sprite_north_west: Sprite2D = $SpriteNorthWest

@export var speed: float = 80
@export var destroy_on_hit: bool = true
@export var touch_damage: int = 1

@export_enum("Player", "Enemy") var team: int = TEAM_PLAYER
@export_flags_2d_physics var team_player_mask: int = 1 << 8
@export_flags_2d_physics var team_enemy_mask: int = 1 << 7

var direction: Vector2 = Vector2.UP

# sprite, flip_h, flip_v; LUT for each 8 directions
@onready var _sprite_data = [
    [sprite_west, true, false],
    [sprite_north_west, true, true],
    [sprite_north, false, true],
    [sprite_north_west, false, true],
    [sprite_west, false, false],
    [sprite_north_west, false, false],
    [sprite_north, false, false],
    [sprite_north_west, true, false],
    [sprite_west, true, false],
]

func _ready() -> void:
    area.area_entered.connect(_on_area_entered)
    match team:
        TEAM_PLAYER:
            area.collision_mask = team_player_mask
        TEAM_ENEMY:
            area.collision_mask = team_enemy_mask

    var direction_angle := fmod(direction.angle(), TAU) / TAU
    # angles returned by angle() are between [-PI, PI), with the center(0) at Vector2.RIGHT
    if direction_angle < 0:
        direction_angle = 1.0 + direction_angle
    direction_angle = floori((direction_angle * 8) + 0.5) # [0, 8]
    var sprite_info = _sprite_data[direction_angle]
    var sprite: Sprite2D = sprite_info[0]
    sprite.visible = true
    sprite.flip_h = sprite_info[1]
    sprite.flip_v = sprite_info[2]

func _physics_process(delta: float) -> void:
    position += direction * speed * delta

func _on_area_entered(area: Area2D) -> void:
    if is_queued_for_deletion():
        return

    # note: bullet teams are handled by the collision mask
    var area_parent: Node = area.get_parent()
    if "damage" in area_parent:
        area_parent.damage(touch_damage)
    
    # TODO: how should it work when the area is a bullet?
    if destroy_on_hit:
        queue_free()

