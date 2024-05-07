extends Node
class_name Game

@onready var floor_generator: FloorGenerator = $FloorGenerator

var room_idx: Vector2i = FloorGenerator.FLOOR_CENTER
var room: Room

func _ready() -> void:
    Global.game = self

func _on_floor_generated(rooms: Array[Array]) -> void:
    room_idx = FloorGenerator.FLOOR_CENTER
    room = rooms[room_idx.x][room_idx.y]

# returns the position the player should be set to for the next room
func goto_room(cardinal: int) -> Vector2:
    var next_idx = room_idx
    var target_player_pos = Vector2.ZERO
    match cardinal:
        Room.NORTH:
            next_idx += Vector2i.UP
            target_player_pos = Vector2(0, 48)
        Room.SOUTH:
            next_idx += Vector2i.DOWN
            target_player_pos = Vector2(0, -48)
        Room.WEST:
            next_idx += Vector2i.LEFT
            target_player_pos = Vector2(48, 0)
        Room.EAST:
            next_idx += Vector2i.RIGHT
            target_player_pos = Vector2(-48, 0)
    
    room.disable()
    
    room_idx = next_idx
    room = floor_generator.rooms[room_idx.x][room_idx.y]
    
    room.enable()

    return target_player_pos
