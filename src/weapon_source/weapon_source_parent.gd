extends Node2D
class_name WeaponSourceParent

@export var default_weapon_scene: PackedScene = preload("res://weapon_source/weapon_source.tscn")
@export var radius: float = 8
@export var start_count: int = 1

@export var use_angle_from_owner: bool = false

@export var target_rotation: float = 0
@export var min_assumed_count: int = 4

@export var index_offset: int = 0

@export_enum("Player", "Enemy") var team: int = 0

var fired_this_frame: bool = false
var source_rotation: float = 0

func _ready() -> void:
    for i in range(start_count):
        add_source()

func _process(delta: float) -> void:
    if !is_equal_approx(fmod(source_rotation, TAU), fmod(target_rotation, TAU)):
        source_rotation = smoothdamp_angle(
            source_rotation,
            target_rotation,
            15,
            delta)
        update_source_offsets()

func _physics_process(delta: float) -> void:
    fired_this_frame = false

func update_sources(delta: float, should_shoot: bool) -> void:
    for source in get_children():
        var target_rotation: Vector2
        if use_angle_from_owner:
            target_rotation = global_position.direction_to(source.global_position)
        else:
            target_rotation = Vector2.from_angle(source_rotation)
        source.update(delta, target_rotation, should_shoot)
        if source.fired_this_frame:
            fired_this_frame = true

func add_source() -> void:
    var new_source: WeaponSource = default_weapon_scene.instantiate()
    new_source.team = team
    add_child(new_source)
    update_source_offsets()

func update_source_offsets() -> void:
    var assumed_count = max(get_child_count(), min_assumed_count)
    var index := 0
    for child in get_children():
        var fraction := (index + index_offset) / float(assumed_count)
        var dir := Vector2.from_angle(source_rotation + (fraction * 2 * PI))
        child.position = dir * radius
        index += 1

func smoothdamp_angle(from: float, to: float, lambda: float, dt: float) -> float:
    return lerp_angle(from, to, 1.0 - exp(-lambda * dt))
