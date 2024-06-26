class_name BulletBurst extends Bullet

@export var speed: float = 80
@export var damp: float = 0.96

var direction: Vector2 = Vector2.UP
var velocity: Vector2 = Vector2.ZERO

func _ready() -> void:
    super._ready()
    velocity = direction * speed

func _physics_process(delta: float) -> void:
    position += velocity * delta
    velocity *= damp
