extends CharacterBody2D
class_name Player

@export_group("Sprite")
@export var face_south_region: Rect2 = Rect2(72, 0, 16, 16)
@export var face_north_region: Rect2 = Rect2(104, 0, 16, 16)
@export var face_horizontal: Rect2 = Rect2(88, 0, 16, 16)

@export_group("Movement")
@export var normal_max_speed: float = 64
# how long does it take to go from 0 speed to max speed
@export var normal_accel_time: float = 0.02
@export var normal_friction: float = 0.85

@export_group("Health & Damage")
@export var max_health: int = 3
@export var health: int = 3
@export var iframe_time: float = 1.0

@export_group("Sound")
@export var sfx_shoot: AudioStream

@onready var _sfx_player: AudioStreamPlayer = $AudioStreamPlayer

@onready var _sprite: Sprite2D = $Sprite
@onready var _source_parent: WeaponSourceParent = $WeaponSourceParent
var _repel_total: Vector2 = Vector2.ZERO

var _iframe_timer: float = 0

func _ready() -> void:
    Global.player = self

func _physics_process(delta: float) -> void:
    if _iframe_timer > 0:
        _iframe_timer -= delta
    
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
    
    _source_parent.update_sources(delta, shoot_dir != Vector2.ZERO)
    
    if _source_parent.fired_this_frame:
        _sfx_player.stream = sfx_shoot
        _sfx_player.play()
    
    if wish_dir:
        velocity += wish_dir.normalized() * delta * normal_max_speed / normal_accel_time
        velocity = velocity.limit_length(normal_max_speed)
    
    velocity += _repel_total
    
    move_and_slide()
    
    velocity *= normal_friction
    
    if is_on_wall():
        try_enter_doors(wish_dir)
    
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

func try_enter_doors(wish_dir: Vector2) -> void:
    if wish_dir.y < 0 and Global.game.room.door_trigger_has(Room.NORTH, self):
        position = Global.game.goto_room(Room.NORTH)
    elif wish_dir.y > 0 and Global.game.room.door_trigger_has(Room.SOUTH, self):
        position = Global.game.goto_room(Room.SOUTH)
    elif wish_dir.x < 0 and Global.game.room.door_trigger_has(Room.WEST, self):
        position = Global.game.goto_room(Room.WEST)
    elif wish_dir.x > 0 and Global.game.room.door_trigger_has(Room.EAST, self):
        position = Global.game.goto_room(Room.EAST)

func repel(amount: Vector2) -> void:
    _repel_total += amount

func try_damage(amount: int) -> void:
    if _iframe_timer > 0:
        return

    health -= amount
    _iframe_timer = iframe_time
    Global.player_damaged.emit()
    if health <= 0:
        Global.player_died.emit()

