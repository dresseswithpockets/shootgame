extends Node2D
class_name Main

@export var music_die: AudioStream
@export var music_loop: AudioStream

@onready var music_player: AudioStreamPlayer = $MusicPlayer
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
    
    # lil hitsleep for a second before playing death animation
    remove_child(Global.game)
    Global.game.queue_free()
    
    # begin death menu fade in
    menu_death.set_process(true)
    menu_death.set_process_input(true)
    menu_death.visible = true
    menu_death.begin_fade_in()

    # wait for text to appear, hacky way to synchronize the beat to the fade in
    music_player.stop()
    await get_tree().create_timer(0.1).timeout

    # play death music
    music_player.stream = music_die
    music_player.play()

func start_again() -> void:
    Global.game = game_prefab.instantiate()
    add_child(Global.game)
    menu_death.set_process(false)
    menu_death.set_process_input(false)
    menu_death.visible = false
    
    # restart normal game music loop
    music_player.stream = music_loop
    music_player.play()
