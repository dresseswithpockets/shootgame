extends Node2D
class_name Room

enum { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT }

@onready var main_tile_layer: TileMapLayer = $MainLayer

@export var north_pos: Vector2i = Vector2i(-1, -8)
@export var south_pos: Vector2i = Vector2i(-1, 7)
@export var west_pos: Vector2i = Vector2i(-8, -1)
@export var east_pos: Vector2i = Vector2i(7, -1)

@export_group("Door Tile Coords")
@export var source_id: int = 1
@export var door_normal_row: int = 0
@export var door_locked_row: int = 1
@export var door_boss_row: int = 2

@export var cell: Vector2i

func set_north() -> void:
    main_tile_layer.set_cell(north_pos, source_id, Vector2i(0, door_normal_row))
    main_tile_layer.set_cell(north_pos + Vector2i.RIGHT, source_id, Vector2i(1, door_normal_row))

func set_west() -> void:
    main_tile_layer.set_cell(west_pos + Vector2i.DOWN, source_id, Vector2i(2, door_normal_row))
    main_tile_layer.set_cell(west_pos, source_id, Vector2i(3, door_normal_row))

func set_south() -> void:
    main_tile_layer.set_cell(south_pos, source_id, Vector2i(4, door_normal_row))
    main_tile_layer.set_cell(south_pos + Vector2i.RIGHT, source_id, Vector2i(5, door_normal_row))

func set_east() -> void:
    main_tile_layer.set_cell(east_pos, source_id, Vector2i(6, door_normal_row))
    main_tile_layer.set_cell(east_pos + Vector2i.DOWN, source_id, Vector2i(7, door_normal_row))
