extends BursterState

var _fired_this_frame_count := 0

func _physics_process(delta: float) -> void:
    burster.source_parent.update_sources(delta, true)
    if burster.source_parent.fired_this_frame:
        _fired_this_frame_count += 1
        if _fired_this_frame_count >= 2:
            state_machine.transition_to("Moving", null)

func enter(_msd) -> void:
    _fired_this_frame_count = 0
