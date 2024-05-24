class_name BursterState extends State

var burster: Burster

func _ready() -> void:
    await owner.ready
    assert(owner is Burster)
    burster = owner
