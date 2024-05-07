extends CharacterBody2D
class_name Player

const SPEED = 80.0

@onready var _source_parent: WeaponSourceParent = $WeaponSourceParent
var _repel_total: Vector2 = Vector2.ZERO

func _physics_process(delta: float) -> void:
    var wish_dir := Vector2(
        Input.get_axis("move_left", "move_right"),
        Input.get_axis("move_up", "move_down"))
    
    var shoot_dir := Vector2(
        Input.get_axis("shoot_left", "shoot_right"),
        Input.get_axis("shoot_up", "shoot_down"))
    
    if shoot_dir:
        _source_parent.target_rotation = shoot_dir.angle()
    
    if wish_dir:
        velocity = wish_dir.normalized() * SPEED
    else:
        velocity = velocity.move_toward(Vector2.ZERO, SPEED)
    
    velocity += _repel_total

    move_and_slide()
    
    # TODO: fix cobblestoning
    
    _repel_total = Vector2.ZERO

func repel(amount: Vector2) -> void:
    _repel_total += amount

