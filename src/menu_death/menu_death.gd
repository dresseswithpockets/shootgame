extends Control
class_name MenuDeath

@export_group("Controls")
@export var start_again_select_label: Label
@export var quit_select_label: Label
@export var you_cant_go_label: Label

@export_group("Fading")
@export var fade_colors: Array[Color] = [
    Color.BLACK, # p8 color 0
    Color(0.114, 0.169, 0.325, 1.0), # p8 color 1
    Color(0.494, 0.145, 0.325, 1.0), # p8 color 2
    Color(0.373, 0.341, 0.310, 1.0), # p8 color 5
    Color.WHITE, # p8 color 7
]
@export var fade_time: float = 2
@export var noquit_time: float = 2

@onready var _noquit_timer: Timer = $NoQuitTimer
var _current_selected: int = -1

func begin_fade_in() -> void:    
    var tween := create_tween()
    tween.tween_method(_tween_quantized_fade_rect, 1.0, 0.0, fade_time)
    await tween.finished
    _current_selected = 0
    quit_select_label.text = " "
    start_again_select_label.text = ">"

func _tween_quantized_fade_rect(t: float) -> void:
    var idx := floori((len(fade_colors) - 1) * (1 - t))
    modulate = fade_colors[idx]

func _input(event: InputEvent) -> void:
    if event.is_action_pressed("ui_left") or event.is_action_pressed("ui_right"):
        toggle_selected_action()
    elif event.is_action_pressed("ui_accept"):
        accept_action()

func accept_action() -> void:
    match _current_selected:
        0:
            accept_start_again()
        1:
            accept_quit()

func accept_start_again() -> void:
    Global.main.start_again()

func accept_quit() -> void:
    if _noquit_timer.is_stopped():
        you_cant_go_label.visible = true
        _noquit_timer.start()

func noquit_timer_finished() -> void:
    you_cant_go_label.visible = false
    _noquit_timer.stop()

func toggle_selected_action() -> void:
    # 0 is start again
    # 1 is quit
    match _current_selected:
        0:
            _current_selected = 1
            start_again_select_label.text = " "
            quit_select_label.text = ">"
        1:
            _current_selected = 0
            quit_select_label.text = " "
            start_again_select_label.text = ">"
