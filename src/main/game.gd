extends Node
class_name Game

@export var sfx_door_enter: AudioStream

@onready var sfx_player: AudioStreamPlayer = $AudioStreamPlayer
@onready var floor_generator: FloorGenerator = $FloorGenerator
@onready var bullets: Node2D = $Bullets

var room_idx: Vector2i = FloorGenerator.FLOOR_CENTER
var room: Room

func _ready() -> void:
    Global.game = self
    Global.camera = $Camera
    Global.enemy_died.connect(_on_enemy_died)

func _on_floor_generated(rooms: Array[Array]) -> void:
    room_idx = FloorGenerator.FLOOR_CENTER
    room = rooms[room_idx.x][room_idx.y]

func _on_enemy_died() -> void:
    if !room.has_any_enemies():
        room.locked = false

func add_bullet(bullet: Node) -> void:
    bullets.add_child(bullet)

func remove_all_bullets() -> void:
    for bullet in bullets.get_children():
        bullet.queue_free()

# TODO: maybe i should have a cluster "wrangler" which automatically sets
#       angular_radius and angular_offset based on the number of bullets in the
#       cluster? As it is now, this behaviour is not 1:1 - the bullets dont
#       adjust radius or offset like multi shots do in the pico8 version
func shoot_bullet_cluster(
    bullet_prefab: PackedScene,
    cluster_size: int,
    position: Vector2,
    direction: Vector2,
    linear_speed: float,
    team: int,
) -> ClusterWrangler:
    var cluster_wrangler := ClusterWrangler.new()
    for cluster_idx in cluster_size:
        var bullet: BulletLinear = bullet_prefab.instantiate()
        bullet.global_position = position
        bullet.linear_speed = linear_speed
        bullet.team = team
        bullet.angular_offset = TAU * cluster_idx / cluster_size
        bullet.direction = direction
        add_bullet(bullet)
        cluster_wrangler.add_bullet(bullet)
    cluster_wrangler.update_bullets()
    add_bullet(cluster_wrangler)
    return cluster_wrangler

func shoot_bullet_linear(
    bullet_prefab: PackedScene,
    position: Vector2,
    direction: Vector2,
    linear_speed: float,
    team: int,
) -> BulletLinear:
    var bullet: BulletLinear = bullet_prefab.instantiate()
    bullet.global_position = position
    bullet.linear_speed = linear_speed
    bullet.team = team
    bullet.direction = direction
    add_bullet(bullet)
    return bullet

# returns the position the player should be set to for the next room
func goto_room(cardinal: int) -> Vector2:
    var next_idx = room_idx
    var target_player_pos = Vector2.ZERO
    match cardinal:
        RoomNormal.NORTH:
            next_idx += Vector2i.UP
            target_player_pos = Vector2(0, 48)
        RoomNormal.SOUTH:
            next_idx += Vector2i.DOWN
            target_player_pos = Vector2(0, -48)
        RoomNormal.WEST:
            next_idx += Vector2i.LEFT
            target_player_pos = Vector2(48, 0)
        RoomNormal.EAST:
            next_idx += Vector2i.RIGHT
            target_player_pos = Vector2(-48, 0)
    
    room.disable()
    remove_all_bullets()
    
    room_idx = next_idx
    room = floor_generator.rooms[room_idx.x][room_idx.y]
    
    room.enable()
    if room.has_any_enemies():
        room.locked = true
    
    sfx_play_door_enter()

    return target_player_pos

func sfx_play_door_enter() -> void:
    sfx_player.stream = sfx_door_enter
    sfx_player.play()
