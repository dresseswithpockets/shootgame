# state machines are hierarchical states themselves
class_name StateMachine extends State

signal transitioned(state_name)

var state: State

func _ready() -> void:
    if get_child_count() == 0:
        return
    
    for child in get_children():
        child.state_machine = self
        child.process_mode = Node.PROCESS_MODE_DISABLED

    state = get_child(0)
    state.process_mode = Node.PROCESS_MODE_INHERIT
    state.enter(null)

func transition_to(target_state_name: String, msg) -> void:
    if not has_node(target_state_name):
        return
    
    state.exit()
    state.process_mode = Node.PROCESS_MODE_DISABLED
    state = get_node(target_state_name)
    state.process_mode = Node.PROCESS_MODE_INHERIT
    state.enter(msg)
    transitioned.emit(state.name)
