class_name BossLilguyStatePhase2 extends BossLilguyState

@export_group("Shooting")
@export var radial_multishot_prefab: PackedScene = preload("res://bullet/linear_bullet_ball.tscn")
@export var radial_multishot_radius: float = 1
@export var radial_multishot_count: int = 8
@export var radial_multishot_arc: float = TAU
@export var radial_multishot_linear_speed: float = 0
@export var radial_multishot_tangent_speed: float = 4
@export var radial_multishot_radius_speed: float = 15
@export var shoot_delay: float = 2

@onready var shoot_timer: float = shoot_delay
var time: float = 0

func _physics_process(delta: float) -> void:
    time += delta

    # should i put this in BossLilguy.damage?
    if boss_lilguy.health <= (boss_lilguy.max_health * 0.333):
        state_machine.transition_to("Phase3", null)

    try_shoot(delta)

func enter(msg) -> void:
    boss_lilguy.position.x = 0

func try_shoot(delta: float) -> void:
    shoot_timer -= delta
    if shoot_timer <= 0:
        shoot_radial()
        shoot_timer = shoot_delay

func shoot_radial() -> void:
    var angle := TAU * 0.1 * time

    var bullet_arc_step := radial_multishot_arc / radial_multishot_count
    for radial_multishot_idx in radial_multishot_count:
        angle += bullet_arc_step
        var direction := Vector2.from_angle(angle)
        var position := boss_lilguy.global_position + direction * radial_multishot_radius
        var bullet: BulletLinear = Global.game.shoot_bullet_angular(
            radial_multishot_prefab,
            position,
            direction,
            radial_multishot_linear_speed,
            radial_multishot_tangent_speed,
            radial_multishot_radius_speed,
            angle,
            Bullet.TEAM_ENEMY)
        bullet.destroy_on_dmg = false
