extends Node2D
class_name Room

enum { UP, DOWN, LEFT, RIGHT }
enum { NORTH, SOUTH, WEST, EAST, CARDINAL_MAX }
enum { DOOR_NONE, DOOR_NORMAL, DOOR_BOSS }

@onready var door_layer: TileMapLayer = $DoorLayer
@onready var north_trigger: Area2D = $NorthTrigger
@onready var south_trigger: Area2D = $SouthTrigger
@onready var west_trigger: Area2D = $WestTrigger
@onready var east_trigger: Area2D = $EastTrigger

@export var north_pos: Vector2i = Vector2i(-1, -8)
@export var south_pos: Vector2i = Vector2i(-1, 7)
@export var west_pos: Vector2i = Vector2i(-8, -1)
@export var east_pos: Vector2i = Vector2i(7, -1)

@export_enum("Normal", "Boss", "Item") var room_type: int = 0

@export_group("Door Tile Coords")
@export var source_id: int = 1
@export var door_normal_row: int = 0
@export var door_locked_row: int = 1
@export var door_boss_row: int = 2

@export var cell: Vector2i

var locked: bool = false:
    get:
        return locked
    set(p):
        locked = p
        update_doors()

@onready var _cell_tuples: Array = [
    DoorCellSet.new(north_pos, north_pos + Vector2i.RIGHT, 0),
    DoorCellSet.new(south_pos, south_pos + Vector2i.RIGHT, 4),
    DoorCellSet.new(west_pos + Vector2i.DOWN, west_pos, 2),
    DoorCellSet.new(east_pos, east_pos + Vector2i.DOWN, 6),
]
var _doors: Array = [DOOR_NONE, DOOR_NONE, DOOR_NONE, DOOR_NONE]
var _locked: bool = false

func disable() -> void:
    visible = false
    process_mode = PROCESS_MODE_DISABLED

func enable() -> void:
    visible = true
    process_mode = PROCESS_MODE_INHERIT

func door_trigger_has(cardinal: int, body: PhysicsBody2D) -> bool:
    var trigger = null
    match cardinal:
        NORTH:
            trigger = north_trigger
        SOUTH:
            trigger = south_trigger
        WEST:
            trigger = west_trigger
        EAST:
            trigger = east_trigger

    return trigger != null and has_door(cardinal) and trigger.overlaps_body(body)

func set_door(cardinal: int, type: int) -> void:
    assert(type == DOOR_NORMAL or type == DOOR_BOSS)
    _doors[cardinal] = type
    update_door(cardinal)

func has_door(cardinal: int) -> bool:
    return _doors[cardinal] != DOOR_NONE

func update_doors() -> void:
    update_door(NORTH)
    update_door(SOUTH)
    update_door(WEST)
    update_door(EAST)

func update_door(cardinal: int) -> void:
    var type = _doors[cardinal]
    var use_source_id = source_id
    var use_row = door_normal_row
    match type:
        DOOR_NONE:
            use_source_id = -1
        DOOR_BOSS:
            use_row = door_boss_row
    if _locked:
        use_row = door_locked_row
    var cellset = _cell_tuples[cardinal]
    door_layer.set_cell(cellset.a, use_source_id, Vector2i(cellset.col, use_row))
    door_layer.set_cell(cellset.b, use_source_id, Vector2i(cellset.col + 1, use_row))

class DoorCellSet:
    var a: Vector2i
    var b: Vector2i
    var col: int
    
    func _init(a: Vector2i, b: Vector2i, start_col: int):
        self.a = a
        self.b = b
        self.col = start_col
