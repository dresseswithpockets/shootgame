extends CharacterBody2D

@export_group("Movement")
@export var normal_max_speed: float = 32
# how long does it take to go from 0 speed to max speed
@export var normal_accel_time: float = 0.02
@export var normal_friction: float = 0.85

@export_group("Health & Damage")
@export var max_health: int = 12
@export var health: int = 12
@export var touch_damage: int = 1

@onready var _hurt_area: Area2D = $HurtArea
var _repel_amount: Vector2 = Vector2.ZERO

func _physics_process(delta: float) -> void:
    var wish_dir := global_position.direction_to(Global.player.global_position)
    velocity += wish_dir * delta * normal_max_speed / normal_accel_time
    velocity = velocity.limit_length(normal_max_speed)
    velocity += _repel_amount
    
    move_and_slide()
    
    velocity *= normal_friction
    
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
        queue_free()
