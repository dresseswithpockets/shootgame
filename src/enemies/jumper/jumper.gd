class_name Jumper extends Enemy

@export_group("Jumping & Movement")
@export var normal_friction: float = 0.96
@export var jump_speed: float = 128
@export var jump_delay: float = 1
@export var jump_player_chance: float = 0.25
@export var jump_perp_chance: float = 0.5

@onready var jump_delay_timer: float = jump_delay

func physics_process(delta: float) -> void:
    jump_delay_timer -= delta
    if jump_delay_timer <= 0:
        velocity = get_jump_dir() * jump_speed
        jump_delay_timer = jump_delay
    
    apply_repel()
    move_and_slide()

    velocity *= normal_friction

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
