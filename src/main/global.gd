extends Node
# big global autoload class used for game-wide signals & references

signal player_damaged
signal player_died
signal enemy_died

var main: Main
var game: Game
var player: Player
var camera: Camera2D
var room_center: Vector2 = Vector2.ZERO
