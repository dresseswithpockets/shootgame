extends Node2D
class_name FloorGenerator

const FLOOR_WIDTH: int = 9
const FLOOR_HEIGHT: int = 9
const FLOOR_CENTER := Vector2i(4, 4)

signal floor_generated(rooms: Array[Array])

@export var floor_depth_factor: float = 3.33
@export var min_rand_bound: int = 4
@export var max_rand_bound: int = 5
@export var max_base_room_count: int = 20

@onready var room_prefab: PackedScene = preload("res://room/room.tscn")
@onready var walker_prefab: PackedScene = preload("res://enemies/walker/walker.tscn")
@onready var jumper_prefab: PackedScene = preload("res://enemies/jumper/jumper.tscn")
@onready var burster_prefab: PackedScene = preload("res://enemies/burster/burster.tscn")

var current_floor_plan: FloorPlan
var rooms: Array[Array] = []

func _ready():
    self.rooms = []
    for x in range(FLOOR_WIDTH):
        var col = []
        for y in range(FLOOR_HEIGHT):
            col.append(null)
        self.rooms.append(col)
    generate_floor(1)

func clear_rooms():
    for x in range(FLOOR_WIDTH):
        for y in range(FLOOR_HEIGHT):
            self.rooms[x][y] = null

func compare_distance_to_center(a: Vector2i, b: Vector2i) -> int:
    var centerf := Vector2(FLOOR_CENTER)
    var distance_a := centerf.distance_squared_to(Vector2(a))
    var distance_b := centerf.distance_squared_to(Vector2(b))
    if distance_a < distance_b:
        return 1
    if distance_a > distance_b:
        return -1
    return 0

func generate_floor(floor_depth: int) -> void:
    for child in get_children():
        remove_child(child)
    
    clear_rooms()

    while !generate_floor_plan(1):
        pass

    var room_neighbors: Array[Vector2i] = []
    for x in range(FLOOR_WIDTH):
        for y in range(FLOOR_HEIGHT):
            var floor_room: FloorRoom = current_floor_plan.rooms[x][y]
            if floor_room == null:
                continue

            var room: Room = room_prefab.instantiate()
            rooms[x][y] = room
            room.room_type = floor_room.room_type
            room.cell = Vector2i(x, y)
            room.name = "Room%d%d" % [x, y]
            
            # add enemies to all rooms except for the boss room, item room, and center
            if room.cell != FLOOR_CENTER and room.room_type == FloorRoom.NORMAL:
                generate_enemies(room)
            
            if FLOOR_CENTER.x == x and FLOOR_CENTER.y == y:
                room.enable()
            else:
                room.disable()
                
            add_child(room)
            
            if current_floor_plan.has_room_at(Vector2i(x - 1, y)):
                room.set_door(Room.WEST, Room.DOOR_NORMAL)
            if current_floor_plan.has_room_at(Vector2i(x + 1, y)):
                room.set_door(Room.EAST, Room.DOOR_NORMAL)
            if current_floor_plan.has_room_at(Vector2i(x, y - 1)):
                room.set_door(Room.NORTH, Room.DOOR_NORMAL)
            if current_floor_plan.has_room_at(Vector2i(x, y + 1)):
                room.set_door(Room.SOUTH, Room.DOOR_NORMAL)
    
    floor_generated.emit(rooms)

var _enemy_gen_corners: Array[Vector2] = [
    Vector2(-46, -46),
    Vector2(46, -46),
    Vector2(-46, 46),
    Vector2(46, 46),
]

func generate_enemies(room: Room) -> void:
    match randi_range(0, 3):
        0:
            generate_enemies_swarm(room)
        1:
            generate_enemies_walkers(room)
        2:
            generate_enemies_walker_pets(room)
        3:
            generate_enemies_burster(room)

func generate_enemies_swarm(room: Room) -> void:
    # generates a handful of jumpers, 1 per corner. Can be 3 or 4
    var count := 3 + randi_range(0, 1)
    _enemy_gen_corners.shuffle()
    for i in count:
        var enemy: Enemy = jumper_prefab.instantiate()
        enemy.position = _enemy_gen_corners[wrapi(i, 0, len(_enemy_gen_corners))]
        room.add_child(enemy)

func generate_enemies_walkers(room: Room) -> void:
    var count := 0
    var chance := randf()
    if chance < 0.5: count = 1
    elif chance <= 0.8: count = 2
    else: count = 3
    _enemy_gen_corners.shuffle()
    for i in count:
        var enemy: Enemy = walker_prefab.instantiate()
        enemy.position = _enemy_gen_corners[wrapi(i, 0, len(_enemy_gen_corners))]
        room.add_child(enemy)

func generate_enemies_walker_pets(room: Room) -> void:
    var count := 0
    var chance := randf()
    if chance <= 0.75: count = 1
    else: count = 2
    for i in count:
        var enemy: Enemy = walker_prefab.instantiate()
        var enemy_pos := _enemy_gen_corners[wrapi(i, 0, len(_enemy_gen_corners))]
        enemy.position = enemy_pos
        room.add_child(enemy)
        
        var dir_to_center := enemy_pos.direction_to(FLOOR_CENTER)
        enemy_pos += dir_to_center * 16
        enemy = jumper_prefab.instantiate()
        enemy.position = enemy_pos
        room.add_child(enemy)

func generate_enemies_burster(room: Room) -> void:
    var enemy: Enemy = burster_prefab.instantiate()
    enemy.position = _enemy_gen_corners.pick_random()
    room.add_child(enemy)

func generate_floor_plan(floor_depth: int) -> bool:
    current_floor_plan = FloorPlan.new()
    
    var min_room_count := int(3.33 * floor_depth + randi_range(min_rand_bound, max_rand_bound))
    min_room_count = min(min_room_count, max_base_room_count)
    
    var min_branch_length: int = min_room_count / 4
    # must have at least 1 dead end for each: boos room & item room
    var min_dead_ends := 2
    
    # center is always enabled
    current_floor_plan.rooms[FLOOR_CENTER.x][FLOOR_CENTER.y] = FloorRoom.new()
    # the center room is always an item room on the first floor
    if floor_depth == 1:
        current_floor_plan.rooms[FLOOR_CENTER.x][FLOOR_CENTER.y].room_type = FloorRoom.ITEM
    
    # visit rooms
    var room_stack: Array[Vector2i] = []
    var room_count = 1
    while room_count < min_room_count:
        # if the center room has too many neighbors, then break out of the loop
        # and scan for new potential cells, instead of pushing the center to the
        # stack.
        var center_neighbors = current_floor_plan.count_neighbors(FLOOR_CENTER)
        if center_neighbors == 4:
            break
        
        # if we ran out of rooms in the stack, then default to going back to the
        # center room for a new bracnh
        room_stack.push_back(FLOOR_CENTER)
        
        var branch_length := 0
        while len(room_stack) > 0:
            if room_count >= min_room_count:
                break
            
            var next_idx: Vector2i = room_stack.pop_back()
            
            # there is a N-in-8 chance (N is branch length) that no adjacent
            # cell is picked, unless:
            # - the current cell is the center cell
            # - or, the center cell has too many neighbors
            # - or, this branch isn't big enough
            if FLOOR_CENTER != next_idx and branch_length >= min_branch_length:
                center_neighbors = current_floor_plan.count_neighbors(FLOOR_CENTER)
                # the chance to skip scales with the length of the branch
                if center_neighbors < 4 and randi_range(0, 8) <= branch_length:
                    break
            
            var require_low_neighbors := next_idx != FLOOR_CENTER
            var potential_next_rooms: Array[Vector2i] = []
            if current_floor_plan.check_adjacent_eligibility(next_idx + Vector2i.LEFT, require_low_neighbors):
                potential_next_rooms.push_back(next_idx + Vector2i.LEFT)
            if current_floor_plan.check_adjacent_eligibility(next_idx + Vector2i.RIGHT, require_low_neighbors):
                potential_next_rooms.push_back(next_idx + Vector2i.RIGHT)
            if current_floor_plan.check_adjacent_eligibility(next_idx + Vector2i.UP, require_low_neighbors):
                potential_next_rooms.push_back(next_idx + Vector2i.UP)
            if current_floor_plan.check_adjacent_eligibility(next_idx + Vector2i.DOWN, require_low_neighbors):
                potential_next_rooms.push_back(next_idx + Vector2i.DOWN)
            
            # this branch's tail is stuck in a corner surrounded by cells
            # already in use or OOB
            if len(potential_next_rooms) == 0:
                break
            
            next_idx = potential_next_rooms.pick_random()
            room_stack.push_back(next_idx)
            current_floor_plan.rooms[next_idx.x][next_idx.y] = FloorRoom.new()
            branch_length += 1
            room_count += 1

    # query the floor plan for all actual dead ends and potential dead end
    # indices
    var real_ends: Array[Vector2i] = []
    var potential_ends: Array[Vector2i] = []
    for x in range(FLOOR_WIDTH):
        for y in range(FLOOR_HEIGHT):
            var room_idx := Vector2i(x, y)
            if current_floor_plan.count_neighbors(room_idx) == 1 and room_idx != FLOOR_CENTER:
                if current_floor_plan.rooms[room_idx.x][room_idx.y]:
                    real_ends.append(room_idx)
                else:
                    real_ends.append(room_idx)
    
    # if there is a dead end deficit, then we will randomly select new ones to
    # add
    var dead_end_deficit = min_dead_ends - len(real_ends)
    if len(potential_ends) < dead_end_deficit:
        return false
    
    for i in range(dead_end_deficit):
        var index := randi_range(0, len(potential_ends) - 1)
        real_ends.append(potential_ends[index])
        potential_ends.remove_at(index)
    
    if len(real_ends) < min_dead_ends:
        return false
    
    # sort the real dead ends by distance from center, and pick the furthest for
    # special rooms
    real_ends.sort_custom(compare_distance_to_center)
    var boss_idx := real_ends[0]
    current_floor_plan.rooms[boss_idx.x][boss_idx.y] = FloorRoom.new()
    current_floor_plan.rooms[boss_idx.x][boss_idx.y].room_type = FloorRoom.BOSS
    # TODO: setup boss room with boss?
    
    var item_idx := real_ends[1]
    current_floor_plan.rooms[item_idx.x][item_idx.y] = FloorRoom.new()
    current_floor_plan.rooms[item_idx.x][item_idx.y].room_type = FloorRoom.ITEM
    # TODO: setup item room with items?
    
    return true


class FloorRoom:
    enum { NORMAL, BOSS, ITEM }
    var room_type: int = NORMAL

class FloorPlan:
    var rooms: Array[Array]
    var boss_room: Vector2i
    var item_room: Vector2i
    
    func _init():
        self.rooms = []
        for x in range(FLOOR_WIDTH):
            var col = []
            for y in range(FLOOR_HEIGHT):
                col.append(null)
            self.rooms.append(col)
    
    func has_room_at(pos: Vector2i) -> bool:
        return pos.x >= 0 and pos.x < FLOOR_WIDTH and \
            pos.y >= 0 and pos.y < FLOOR_HEIGHT and \
            self.rooms[pos.x][pos.y]
    
    func neighbors_at(pos: Vector2i, buffer: Array[Vector2i]) -> void:
        if pos.x > 0 and self.rooms[pos.x-1][pos.y]: buffer.append(pos + Vector2i.LEFT)
        if pos.x < FLOOR_WIDTH - 1 and self.rooms[pos.x+1][pos.y]: buffer.append(pos + Vector2i.RIGHT)
        if pos.y > 0 and self.rooms[pos.x][pos.y-1]: buffer.append(pos + Vector2i.UP)
        if pos.y < FLOOR_HEIGHT - 1 and self.rooms[pos.x][pos.y+1]: buffer.append(pos + Vector2i.DOWN)
    
    func count_neighbors(pos: Vector2i) -> int:
        var count := 0
        if pos.x > 0 and self.rooms[pos.x-1][pos.y]: count += 1
        if pos.x < FLOOR_WIDTH - 1 and self.rooms[pos.x+1][pos.y]: count += 1
        if pos.y > 0 and self.rooms[pos.x][pos.y-1]: count += 1
        if pos.y < FLOOR_HEIGHT - 1 and self.rooms[pos.x][pos.y+1]: count += 1
        return count
    
    func check_adjacent_eligibility(pos: Vector2i, require_low_neighbors: bool) -> bool:
        if pos.x < 0 or pos.x >= FLOOR_WIDTH or pos.y < 0 or pos.y >= FLOOR_HEIGHT:
            return false
        if self.rooms[pos.x][pos.y]:
            return false
        if require_low_neighbors and self.count_neighbors(pos) > 1:
            return false
        return true
