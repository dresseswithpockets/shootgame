extends Node2D
class_name Main

func _input(event: InputEvent) -> void:
    if event.is_action_pressed("pause"):
        get_tree().paused = !get_tree().paused
        $PausedContainer.visible = get_tree().paused

func _ready() -> void:
    Global.main = self
