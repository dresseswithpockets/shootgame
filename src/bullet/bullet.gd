class_name Bullet extends Node2D

enum { TEAM_PLAYER, TEAM_ENEMY }

@onready var area: Area2D = $Area2D

@export_enum("Player", "Enemy") var team: int = TEAM_PLAYER

@export var destroy_on_hit: bool = true
@export var touch_damage: int = 1
@export var lifetime: float = 32

var _lifetime_timer: SceneTreeTimer

func _ready() -> void:
    area.area_entered.connect(_on_area_entered)
    _lifetime_timer = get_tree().create_timer(lifetime)
    _lifetime_timer.timeout.connect(queue_free)

func _on_area_entered(area: Area2D) -> void:
    if is_queued_for_deletion():
        return

    var area_parent: Node = area.get_parent()
    if not "team" in area_parent or area_parent.team == team:
        return
    if "damage" in area_parent:
        area_parent.damage(touch_damage)
    if "try_damage" in area_parent:
        area_parent.try_damage(touch_damage)
    
    # TODO: how should it work when the area is a bullet?
    if destroy_on_hit:
        queue_free()
