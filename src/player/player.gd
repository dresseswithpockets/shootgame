extends CharacterBody2D
class_name Player

@export_group("Sprite")
@export var face_south_region: Rect2 = Rect2(72, 0, 16, 16)
@export var face_north_region: Rect2 = Rect2(104, 0, 16, 16)
@export var face_horizontal: Rect2 = Rect2(88, 0, 16, 16)

@export_group("Movement")
@export var base_speed: float = 80
@export var accel: float = 1600
@export var decel: float = 800

@onready var _sprite: Sprite2D = $Sprite
@onready var _source_parent: WeaponSourceParent = $WeaponSourceParent
var _repel_total: Vector2 = Vector2.ZERO

func _physics_process(delta: float) -> void:
    var wish_dir := Vector2(
        Input.get_axis("move_left", "move_right"),
        Input.get_axis("move_up", "move_down"))
    
    var shoot_dir := Vector2(
        Input.get_axis("shoot_left", "shoot_right"),
        Input.get_axis("shoot_up", "shoot_down"))
    
    var flip_dir := shoot_dir if shoot_dir else wish_dir
    if flip_dir:
        _update_sprite(flip_dir)
    
    if shoot_dir:
        _source_parent.target_rotation = shoot_dir.angle()
    
    if wish_dir:
        velocity += wish_dir.normalized() * accel * delta
        velocity = velocity.limit_length(base_speed)
    else:
        velocity = velocity.move_toward(Vector2.ZERO, decel * delta)
    
    velocity += _repel_total

    move_and_slide()
    
    if is_on_wall():
        try_enter_doors()
    
    # TODO: fix cobblestoning
    
    _repel_total = Vector2.ZERO

func _update_sprite(flip_dir: Vector2) -> void:
    if flip_dir.y < 0:
        _sprite.region_rect = face_north_region
    elif flip_dir.y > 0:
        _sprite.region_rect = face_south_region
    
    _sprite.flip_h = flip_dir.x > 0
    if flip_dir.x != 0:
        _sprite.region_rect = face_horizontal

func repel(amount: Vector2) -> void:
    _repel_total += amount

func try_enter_doors() -> void:
    for cardinal in range(Room.CARDINAL_MAX):
        if Global.game.room.door_trigger_has(cardinal, self):
            position = Global.game.goto_room(cardinal)
