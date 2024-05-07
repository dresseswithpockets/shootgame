extends Area2D
class_name Repeller

@export var repel_force: float = 1.0
@onready var circle_shape: CircleShape2D = $CollisionShape2D.shape

func radius() -> float:
    return circle_shape.radius

func _physics_process(_delta: float) -> void:
    for area in get_overlapping_areas():
        # the Repel area is masked to only other repellers, so its assumed
        # to have a parent that can receive "repel"
        var their_parent: Node2D = area.get_parent()
        var direction := global_position.direction_to(area.global_position)
        var distance := global_position.distance_to(area.global_position)
        
        var total_radius: float = area.radius() + radius()
        var power := (total_radius - distance) / total_radius
        
        var repel_amount := direction * power * repel_force
        if their_parent.has_method("repel"):
            their_parent.repel(direction * power * repel_force)
        elif their_parent is RigidBody2D:
            their_parent.linear_velocity += repel_amount
