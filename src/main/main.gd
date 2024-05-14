extends Node2D
class_name Main

@onready var menu_death: MenuDeath = $MenuDeathParent/MenuDeath

var game_prefab: PackedScene = preload("res://main/game.tscn")

func _input(event: InputEvent) -> void:
    if event.is_action_pressed("pause"):
        get_tree().paused = !get_tree().paused
        $PausedContainer.visible = get_tree().paused

func _ready() -> void:
    Global.main = self
    Global.player_died.connect(_on_player_died)
    menu_death.set_process(false)
    menu_death.set_process_input(false)

func _on_player_died() -> void:
    # when the player dies, we will completely remove the Game prefab, and start the player
    # death screen
    remove_child(Global.game)
    Global.game.queue_free()
    menu_death.set_process(true)
    menu_death.set_process_input(true)
    menu_death.visible = true
    menu_death.begin_fade_in()

func start_again() -> void:
    Global.game = game_prefab.instantiate()
    add_child(Global.game)
    menu_death.set_process(false)
    menu_death.set_process_input(false)
    menu_death.visible = false
