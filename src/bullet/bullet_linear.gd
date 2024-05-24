class_name BulletLinear extends Bullet

@onready var sprite_north: Sprite2D = $SpriteNorth
@onready var sprite_west: Sprite2D = $SpriteWest
@onready var sprite_north_west: Sprite2D = $SpriteNorthWest

@export var speed: float = 80

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
