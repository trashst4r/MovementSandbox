cmake_minimum_required(VERSION 3.20)
project(MovementSandbox LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MovementSandbox
    src/main.cpp
)

target_include_directories(MovementSandbox PRIVATE
    "C:/raylib/raylib/src"
)

target_link_directories(MovementSandbox PRIVATE
    "C:/raylib/raylib/src"
)

target_link_libraries(MovementSandbox PRIVATE
    raylib
    winmm
    gdi32
    user32
    shell32
    opengl32
)