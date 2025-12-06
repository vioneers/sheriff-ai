cmake_minimum_required(VERSION 3.15)
project(Sheriff-AI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

set(ENGINE_SOURCES
    src/engine.cpp
    src/board.cpp
    src/piece.cpp
    src/move.cpp
	src/evaluationbar.cpp
)

add_executable(sheriff_ai
    src/main.cpp
    ${ENGINE_SOURCES}
)

add_executable(sheriff_ai_lichess
    src/main_lichess.cpp
    ${ENGINE_SOURCES}
)
