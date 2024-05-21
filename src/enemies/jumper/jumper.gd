class_name Jumper extends CharacterBody2D

@export_group("Health & Damage")
@export var max_health: int = 12
@export var health: int = 12
@export var touch_damage: int = 1

@onready var _hurt_area: Area2D = $HurtArea
var _repel_amount: Vector2 = Vector2.ZERO

@export_group("Jumping & Movement")
@export var normal_friction: float = 0.96
@export var jump_speed: float = 128
@export var jump_delay: float = 1
@export var jump_player_chance: float = 0.25
@export var jump_perp_chance: float = 0.5

@onready var jump_delay_timer: float = jump_delay

func _physics_process(delta: float) -> void:
    jump_delay_timer -= delta
    if jump_delay_timer <= 0:
        velocity = get_jump_dir() * jump_speed
        jump_delay_timer = jump_delay
    
    velocity += _repel_amount

    move_and_slide()

    velocity *= normal_friction
    
    # try damaging anything we're intersecting every frame. This way, any other actors with
    # iframes will still get damaged again if they're still inside us
    for area in _hurt_area.get_overlapping_areas():
        var parent: Node = area.get_parent()
        if "try_damage" in parent:
            parent.try_damage(touch_damage)
    
    _repel_amount = Vector2.ZERO

func get_jump_dir() -> Vector2:
    var choice = randf()
    if choice < jump_player_chance:
        # jump towards the player
        return global_position.direction_to(Global.player.global_position)
    
    var dir: Vector2
    if choice < jump_perp_chance:
        # jump perpindicular to the player
        dir = global_position.direction_to(Global.player.global_position)
    else:
        # jump perpindicular to center of room
        dir = global_position.direction_to(Global.room_center)
    
    var orthogonal = dir.orthogonal()
    if randf() < 0.5:
        # 50% chance to flip/reflect the orthogonal across dir
        orthogonal = orthogonal.reflect(dir)
    
    # we want them to move "inwards" towards the center so lerp to the original dir
    return orthogonal.slerp(dir, 0.5)

func repel(amount: Vector2) -> void:
    _repel_amount += amount

func damage(amount: int) -> void:
    health -= amount
    if health <= 0:
        queue_free()
