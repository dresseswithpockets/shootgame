extends Node2D
class_name WeaponSourceParent

@onready var weapon_source_scene: PackedScene = preload("res://weapon_source/weapon_source.tscn")
@export var radius: float = 8
@export var start_count: int = 1

@export var target_rotation: float = 0

func _ready() -> void:
    for i in range(start_count):
        add_source()

func _process(delta: float) -> void:
    if !is_equal_approx(fmod(rotation, 360), fmod(target_rotation, 360)):
        rotation = smoothdamp_angle(
            rotation,
            target_rotation,
            10,
            delta)

func add_source() -> void:
    var new_source: WeaponSource = weapon_source_scene.instantiate()
    add_child(new_source)
    update_source_offsets()

func update_source_offsets() -> void:
    var assumed_count = max(get_child_count(), 4)
    var index := 0
    for child in get_children():
        var fraction := index / float(assumed_count)
        var dir := Vector2.from_angle(fraction * 2 * PI)
        child.position = dir * radius
        index += 1

func smoothdamp_angle(from: float, to: float, lambda: float, dt: float) -> float:
    return lerp_angle(from, to, 1.0 - exp(-lambda * dt))