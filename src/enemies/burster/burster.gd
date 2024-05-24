class_name Burster extends Enemy

@export var max_shoot_distance: float = 70
@export var min_shoot_distance: float = 40
@export var source_turn_speed: float = 0.5

# burster has a few states:
# if the player is too far away, move towards player
# once close enough, shoot player
# if too close to the player, step back

@export_group("Movement")
@export var normal_max_speed: float = 32
# how long does it take to go from 0 speed to max speed
@export var normal_accel_time: float = 0.02
@export var normal_friction: float = 0.85

@onready var source_parent: WeaponSourceParent = $WeaponSourceParent

func physics_process(delta: float) -> void:
    source_parent.target_rotation = global_position.direction_to(Global.player.global_position).angle()
    # source_parent.target_rotation += source_turn_speed * TAU * delta
    
    apply_repel()
    move_and_slide()
    
    velocity *= normal_friction
