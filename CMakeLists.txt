cmake_minimum_required(VERSION 3.28)
project(shootgame C)

set(CMAKE_C_STANDARD 23)

include_directories(include)
link_directories(lib/windows-msvc)

add_executable(shootgame src/main.c src/dungeon.c src/dynamic_array.c src/entity.c src/game.c src/input.c src/sprite.c)

target_precompile_headers(shootgame PUBLIC src/pch.h)
target_link_libraries(shootgame raylib winmm gdi32)
