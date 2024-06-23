class_name Enemy extends CharacterBody2D

enum { TEAM_PLAYER, TEAM_ENEMY }

@export_enum("Player", "Enemy") var team: int = TEAM_ENEMY

@export_group("Health & Damage")
@export var max_health: int = 12
@export var health: int = 12
@export var touch_damage: int = 1

@onready var _hurt_area: Area2D = $HurtArea
var _repel_amount: Vector2 = Vector2.ZERO

func physics_process(delta: float) -> void:
    pass

func apply_repel() -> void:
    velocity += _repel_amount

func _physics_process(delta: float) -> void:
    physics_process(delta)
    
    # try damaging anything we're intersecting every frame. This way, any other actors with
    # iframes will still get damaged again if they're still inside us
    for area in _hurt_area.get_overlapping_areas():
        var parent: Node = area.get_parent()
        if "try_damage" in parent:
            parent.try_damage(touch_damage)
    
    _repel_amount = Vector2.ZERO

func repel(amount: Vector2) -> void:
    _repel_amount += amount

func damage(amount: int) -> void:
    health -= amount
    if health <= 0:
        get_parent().remove_child(self)
        Global.enemy_died.emit()
        queue_free()
