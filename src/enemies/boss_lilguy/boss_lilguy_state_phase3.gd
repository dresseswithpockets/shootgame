# TODO: i really need to make phases more modular than this...
#       there is so much code thats just copy pasted between each of the phases here
#       i totally could just have each of these be like "phase executors" or something
#       with each phase being nothing more than a publisher for a physics update
class_name BossLilguyStatePhase3 extends BossLilguyState

@export_group("Shooting")
@export var shoot_delay: float = 2
@export var multishot_prefab: PackedScene = preload("res://bullet/linear_bullet_ball.tscn")
@export var multishot_radius: float = 1
@export var multishot_count: int = 3
@export var multishot_arc: float = 0.3 * TAU
@export var multishot_linear_speed: float = 40
@export var multishot_cluster_size: int = 3

@export_subgroup("Spreadshot")
@export var spreadshot_prefab: PackedScene = preload("res://bullet/linear_bullet.tscn")
@export var spreadshot_linear_speed: float = 60
@export var spreadshot_chance: float = 0.2
@export var spreadshot_count: int = 7
@export var spreadshot_fire_delay: float = 0.06666666666666666666666666666667

@export_subgroup("Radial Multishot")
@export var radial_multishot_prefab: PackedScene = preload("res://bullet/linear_bullet_ball.tscn")
@export var radial_multishot_radius: float = 1
@export var radial_multishot_count: int = 8
@export var radial_multishot_arc: float = TAU
@export var radial_multishot_linear_speed: float = 0
@export var radial_multishot_tangent_speed: float = 4
@export var radial_multishot_radius_speed: float = 15

@onready var shoot_timer: float = shoot_delay
@onready var spreadshot_fire_timer: float = spreadshot_fire_delay
@onready var spreadshot_pending_count: int = 0
var time: float = 0

func _physics_process(delta: float) -> void:
    time += delta
    try_shoot(delta)

func enter(msg) -> void:
    boss_lilguy.position.x = 0

func try_shoot(delta: float) -> void:
    if spreadshot_pending_count == 0:
        shoot_timer -= delta
        if shoot_timer <= 0:
            if randf() <= spreadshot_chance:
                spreadshot_pending_count = spreadshot_count
            else:
                shoot_baseshot()
            shoot_radial()
            shoot_timer = shoot_delay

    if spreadshot_pending_count > 0:
        spreadshot_fire_timer -= delta
        if spreadshot_fire_timer <= 0:
            shoot_spreadshot()
            spreadshot_fire_timer = spreadshot_fire_delay
            spreadshot_pending_count -= 1

func shoot_spreadshot() -> void:
    var t := (float(spreadshot_pending_count) / float(spreadshot_count))
    var right_angle := lerp_angle(0, deg_to_rad(70), 1 - t)
    var left_angle := lerp_angle(deg_to_rad(110), PI, t)
    Global.game.shoot_bullet_linear(spreadshot_prefab, boss_lilguy.global_position, Vector2.from_angle(right_angle), spreadshot_linear_speed, Bullet.TEAM_ENEMY)
    Global.game.shoot_bullet_linear(spreadshot_prefab, boss_lilguy.global_position, Vector2.from_angle(left_angle), spreadshot_linear_speed, Bullet.TEAM_ENEMY)

func shoot_baseshot() -> void:
    var direction := boss_lilguy.global_position.direction_to(Global.player.global_position)
    direction = direction.rotated(-multishot_arc / 2)

    var bullet_arc_step := multishot_arc / (multishot_count + 1)
    for multishot_idx in multishot_count:
        direction = direction.rotated(bullet_arc_step)
        var position := boss_lilguy.global_position + direction * multishot_radius
        Global.game.shoot_bullet_cluster(multishot_prefab, multishot_cluster_size, position, direction, multishot_linear_speed, Bullet.TEAM_ENEMY)

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
