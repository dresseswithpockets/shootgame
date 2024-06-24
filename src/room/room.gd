class_name Room extends Node2D

@onready var enemies: Node2D = $Enemies

@export_enum("Normal", "Boss", "Item") var room_type: int = 0
@export var cell: Vector2i

func disable() -> void:
    visible = false
    process_mode = PROCESS_MODE_DISABLED

func enable() -> void:
    visible = true
    process_mode = PROCESS_MODE_INHERIT

func add_enemy(enemy: Enemy) -> void:
    enemies.add_child(enemy)

func has_any_enemies() -> bool:
    return enemies.get_child_count() > 0
