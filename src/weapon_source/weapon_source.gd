extends Node2D
class_name WeaponSource

@onready var _linear_bullet_prefab: PackedScene = preload("res://bullet/linear_bullet.tscn")
var delay_timer: float = 0
@export var delay_time: float = 0.2

func update(delta: float, dir: Vector2, should_shoot: bool) -> void:
    if delay_timer > 0:
        delay_timer -= delta
    
    if should_shoot and delay_timer <= 0:
        var bullet = _linear_bullet_prefab.instantiate()
        bullet.team = Bullet.TEAM_PLAYER
        bullet.global_position = global_position
        bullet.direction = dir
        Global.game.add_bullet(bullet)
        delay_timer = delay_time
