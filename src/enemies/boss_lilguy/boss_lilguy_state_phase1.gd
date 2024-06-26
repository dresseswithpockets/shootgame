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

var last_x: float = 0
@onready var next_x: float = randf_range(bound_left, bound_right)
@onready var move_time: float = floori(randf_range(move_time_min, move_time_max))
@onready var move_timer: float = move_time

@onready var shoot_timer: float = shoot_delay

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
    shoot_timer -= delta
    if shoot_timer <= 0:
        shoot_baseshot()
        shoot_timer = shoot_delay

# TODO: maybe i should have a cluster "wrangler" which automatically sets
#       angular_radius and angular_offset based on the number of bullets in the
#       cluster? As it is now, this behaviour is not 1:1 - the bullets dont
#       adjust radius or offset like multi shots do in the pico8 version
func shoot_cluster(bullet_prefab: PackedScene, cluster_size: int, position: Vector2, direction: Vector2, linear_speed: float, team: int):
    for cluster_idx in cluster_size:
        var bullet: BulletLinear = bullet_prefab.instantiate()
        bullet.global_position = position
        bullet.linear_speed = linear_speed
        bullet.team = team
        bullet.angular_offset = TAU * cluster_idx / cluster_size
        bullet.direction = direction
        Global.game.add_bullet(bullet)

func shoot_baseshot() -> void:
    var direction := boss_lilguy.global_position.direction_to(Global.player.global_position)
    direction = direction.rotated(-multishot_arc / 2)

    var bullet_arc_step := multishot_arc / (multishot_count + 1)
    for multishot_idx in multishot_count:
        direction = direction.rotated(bullet_arc_step)
        var position := boss_lilguy.global_position + direction * multishot_radius
        shoot_cluster(multishot_prefab, multishot_cluster_size, position, direction, multishot_linear_speed, Bullet.TEAM_ENEMY)
