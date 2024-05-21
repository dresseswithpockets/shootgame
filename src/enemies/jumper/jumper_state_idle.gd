extends JumperState

var timer: float = 0

func _physics_process(delta: float) -> void:
    timer -= delta
    if timer <= 0:
        pass

func _enter(_msg) -> void:
    timer = jumper.jump_delay_time
