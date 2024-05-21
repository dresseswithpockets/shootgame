class_name JumperState extends State

var jumper: Jumper

func _ready() -> void:
    await owner.ready
    assert(owner is Jumper)
    jumper = owner
