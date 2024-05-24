class_name WeaponSource extends Node2D

@export var bullet_prefab: PackedScene = preload("res://bullet/linear_bullet.tscn")
@export var delay_time: float = 0.2
var delay_timer: float = 0

var fired_this_frame: bool = false

func _physics_process(delta: float) -> void:
    fired_this_frame = false

func fire(dir: Vector2) -> void:
    var bullet = bullet_prefab.instantiate()
    bullet.team = Bullet.TEAM_PLAYER
    bullet.global_position = global_position
    bullet.direction = dir
    Global.game.add_bullet(bullet)

func update(delta: float, dir: Vector2, should_shoot: bool) -> void:
    if delay_timer > 0:
        delay_timer -= delta
    
    if should_shoot and delay_timer <= 0:
        fire(dir)
        delay_timer = delay_time
        fired_this_frame = true
