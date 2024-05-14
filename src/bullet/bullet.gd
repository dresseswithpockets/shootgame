extends Node2D
class_name Bullet

enum { TEAM_PLAYER, TEAM_ENEMY }

@onready var area: Area2D = $Area2D

@export var speed: float = 80
@export var destroy_on_hit: bool = true
@export var touch_damage: int = 1

@export_enum("Player", "Enemy") var team: int = TEAM_PLAYER
@export_flags_2d_physics var team_player_mask: int = 1 << 8
@export_flags_2d_physics var team_enemy_mask: int = 1 << 7

var direction: Vector2 = Vector2.UP

func _ready() -> void:
    area.area_entered.connect(_on_area_entered)
    match team:
        TEAM_PLAYER:
            area.collision_mask = team_player_mask
        TEAM_ENEMY:
            area.collision_mask = team_enemy_mask

func _physics_process(delta: float) -> void:
    position += direction * speed * delta

func _on_area_entered(area: Area2D) -> void:
    if is_queued_for_deletion():
        return

    # note: bullet teams are handled by the collision mask
    var area_parent: Node = area.get_parent()
    if "damage" in area_parent:
        area_parent.damage(touch_damage)
    
    # TODO: how should it work when the area is a bullet?
    if destroy_on_hit:
        queue_free()
