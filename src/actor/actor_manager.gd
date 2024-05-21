extends Node

func _physics_process(delta: float) -> void:
    var repellers: Array[Node] = get_tree().get_nodes_in_group("repel")
    for repeller in repellers:
        for other in repellers:
            if other == repeller or \
                not repeller.pushes or \
                not other.pushes:
                continue
            
            repel_actors(repeller, other)

func repel_actors(a: Repeller, b: Repeller) -> void:
    pass
