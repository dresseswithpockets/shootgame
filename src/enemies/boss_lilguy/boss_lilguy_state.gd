class_name BossLilguyState extends State

# god i wish generics existed in gdscript...
var boss_lilguy: BossLilguy

func _ready() -> void:
    await owner.ready
    assert(owner is BossLilguy)
    boss_lilguy = owner
    ready()

func ready() -> void:
    pass
