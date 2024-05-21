extends CharacterBody2D
class_name SimpleBody

@export var damp_speed: float = 1

var _repel_amount: Vector2 = Vector2.ZERO

func _physics_process(delta: float) -> void:
    velocity = velocity.move_toward(Vector2.ZERO, damp_speed * delta)
    velocity += _repel_amount
    move_and_slide()
    _repel_amount = Vector2.ZERO

func repel(amount: Vector2) -> void:
    _repel_amount += amount
