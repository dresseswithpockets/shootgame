extends BursterState

func _physics_process(delta: float) -> void:
    var global_position = burster.global_position
    var dist_to_player = global_position.distance_to(Global.player.global_position)
    var wish_dir = Vector2.ZERO
    if dist_to_player > burster.max_shoot_distance:
        # move towards the player
        wish_dir = global_position.direction_to(Global.player.global_position)
    elif dist_to_player < burster.min_shoot_distance:
        # move away from the player
        wish_dir = -global_position.direction_to(Global.player.global_position)
    else:
        # we're close enough to shoot, so start shooting next frame
        state_machine.transition_to("Shooting", null)

    burster.velocity += wish_dir * delta * burster.normal_max_speed / burster.normal_accel_time
    burster.velocity = burster.velocity.limit_length(burster.normal_max_speed)
