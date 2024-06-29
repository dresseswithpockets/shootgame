class_name BulletLinear extends Bullet

@export var linear_speed: float = 80
@export var tangent_speed: float = 0
@export var tangent_accel: float = 0
@export var angular_radius: float = 0
@export var angular_radius_speed: float = 0
@export var angular_offset: float = 0

@onready var sprite_north: Sprite2D = $AngularOrigin/SpriteNorth
@onready var sprite_west: Sprite2D = $AngularOrigin/SpriteWest
@onready var sprite_north_west: Sprite2D = $AngularOrigin/SpriteNorthWest
@onready var angular_origin: Node2D = $AngularOrigin
@onready var angular_rotation: float = angular_offset

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
    super._ready()
    
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
    position += direction * linear_speed * delta
    angular_radius += angular_radius_speed * delta
    tangent_speed += tangent_accel * delta
    # linear velocity to angular velocity: w=v/r
    if angular_radius > 0:
        var angular_speed = tangent_speed / angular_radius
        angular_rotation += angular_speed * delta
        angular_origin.position.x = cos(angular_rotation) * angular_radius
        angular_origin.position.y = sin(angular_rotation) * angular_radius
