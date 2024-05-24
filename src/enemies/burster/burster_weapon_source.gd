extends WeaponSource

# the burster weapon fires sort of like a minigun - each barrel fires as it
# passes over the direction that the enemy wants to shoot

var _last_dot := -1.0
var _first := true

func _physics_process(delta: float) -> void:
    fired_this_frame = false

func update(delta: float, dir: Vector2, should_shoot: bool) -> void:
    if not should_shoot:
        return

    var dir_from_parent: Vector2 = get_parent().global_position.direction_to(global_position)
    var dir_dot = dir_from_parent.dot(dir)
    if not _first and _last_dot <= 0 and dir_dot >= 0:
        # we just passed over dir, we can shoot
        var bullet = _linear_bullet_prefab.instantiate()
        bullet.team = Bullet.TEAM_ENEMY
        bullet.global_position = global_position
        bullet.direction = dir
        Global.game.add_bullet(bullet)
        fired_this_frame = true
    
    _last_dot = dir_dot
    _first = false
