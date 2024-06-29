class_name BossLilguyStatePhase2 extends BossLilguyState

@export_group("Shooting")
@export var multishot_prefab: PackedScene = preload("res://bullet/linear_bullet_ball.tscn")
@export var multishot_radius: float = 1
@export var multishot_count: int = 8
@export var multishot_arc: float = TAU
@export var multishot_linear_speed: float = 0
@export var multishot_tangent_speed: float = 4
@export var multishot_radius_speed: float = 15
@export var shoot_delay: float = 1.5

@onready var shoot_timer: float = shoot_delay
var time: float = 0

func ready() -> void:
    pass

func _physics_process(delta: float) -> void:
    # should i put this in BossLilguy.damage?
    if boss_lilguy.health <= (boss_lilguy.max_health * 0.333):
        state_machine.transition_to("Phase3", null)

    try_shoot(delta)

func enter(msg) -> void:
    boss_lilguy.position.x = 0

func try_shoot(delta: float) -> void:
    time += delta
    shoot_timer -= delta
    if shoot_timer <= 0:
        shoot_baseshot()
        shoot_timer = shoot_delay

func shoot_baseshot() -> void:
    var angle := TAU * 0.1 * time

    var bullet_arc_step := multishot_arc / multishot_count
    for multishot_idx in multishot_count:
        angle += bullet_arc_step
        var direction := Vector2.from_angle(angle)
        var position := boss_lilguy.global_position + direction * multishot_radius
        var bullet: BulletLinear = Global.game.shoot_bullet_angular(
            multishot_prefab,
            position,
            direction,
            multishot_linear_speed,
            multishot_tangent_speed,
            multishot_radius_speed,
            angle,
            Bullet.TEAM_ENEMY)
        bullet.destroy_on_dmg = false
