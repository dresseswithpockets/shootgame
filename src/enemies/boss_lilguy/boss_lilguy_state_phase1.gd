class_name BossLilguyStatePhase1 extends BossLilguyState

@export_group("Movement")
@export var bound_left: float = -56
@export var bound_right: float = 56
@export var move_time_min: float = 0.5
@export var move_time_max: float = 2

@export_group("Shooting")
@export var multishot_prefab: PackedScene = preload("res://bullet/linear_bullet_ball.tscn")
@export var multishot_radius: float = 1
@export var multishot_count: int = 3
@export var multishot_arc: float = 0.3 * TAU
@export var multishot_linear_speed: float = 60
@export var multishot_cluster_size: int = 3
@export var shoot_delay: float = 1.5

@export_subgroup("Spreadshot")
@export var spreadshot_prefab: PackedScene = preload("res://bullet/linear_bullet.tscn")
@export var spreadshot_linear_speed: float = 60
@export var spreadshot_chance: float = 0.2
@export var spreadshot_count: int = 7
@export var spreadshot_fire_delay: float = 0.06666666666666666666666666666667

var last_x: float = 0
@onready var next_x: float = randf_range(bound_left, bound_right)
@onready var move_time: float = floori(randf_range(move_time_min, move_time_max))
@onready var move_timer: float = move_time

@onready var shoot_timer: float = shoot_delay
@onready var spreadshot_fire_timer: float = spreadshot_fire_delay
@onready var spreadshot_pending_count: int = 0

func ready() -> void:
    last_x = boss_lilguy.position.x

func _physics_process(delta: float) -> void:
    # should i put this in BossLilguy.damage?
    if boss_lilguy.health <= (boss_lilguy.max_health * 0.666):
        state_machine.transition_to("Phase2", null)
    
    # I could just use a tweener instead...
    var x := lerpf(next_x, last_x, move_timer / move_time)
    boss_lilguy.position.x = x
    
    move_timer -= delta
    if move_timer <= 0:
        last_x = next_x
        next_x = randf_range(bound_left, bound_right)
        move_time = randf_range(move_time_min, move_time_max)
        move_timer = move_time

    try_shoot(delta)

func enter(msg) -> void:
    pass

func try_shoot(delta: float) -> void:
    if spreadshot_pending_count == 0:
        shoot_timer -= delta
        if shoot_timer <= 0:
            if randf() <= spreadshot_chance:
                spreadshot_pending_count = spreadshot_count
            else:
                shoot_baseshot()
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
