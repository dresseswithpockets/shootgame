extends Node2D

@onready var area: Area2D = $Area2D

@export var speed: float = 80
@export var destroy_on_hit: bool = true

var direction: Vector2 = Vector2.UP

func _ready() -> void:
    area.area_entered.connect(_on_area_entered)
    area.body_entered.connect(_on_body_entered)

func _physics_process(delta: float) -> void:
    position += direction * speed * delta

func _on_area_entered(area: Area2D) -> void:
    if is_queued_for_deletion():
        return

    # todo: damage other bullets
    pass

func _on_body_entered(body: PhysicsBody2D) -> void:
    if is_queued_for_deletion():
        return
    
    # todo: do damage

    if destroy_on_hit:
        queue_free()
