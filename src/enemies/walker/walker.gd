class_name Walker extends Enemy

@export_group("Movement")
@export var normal_max_speed: float = 32
# how long does it take to go from 0 speed to max speed
@export var normal_accel_time: float = 0.02
@export var normal_friction: float = 0.85

func physics_process(delta: float) -> void:
    var wish_dir := global_position.direction_to(Global.player.global_position)
    velocity += wish_dir * delta * normal_max_speed / normal_accel_time
    velocity = velocity.limit_length(normal_max_speed)

    apply_repel()
    move_and_slide()
    
    velocity *= normal_friction
