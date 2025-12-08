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

add_executable(sheriff_ai_perft
    src/perft.cpp
    ${ENGINE_SOURCES}
)

target_compile_definitions(sheriff_ai
    PRIVATE
	    # If the debug configuration pass the DEBUG define to the compiler
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(sheriff_ai_lichess
    PRIVATE
	    # If the debug configuration pass the DEBUG define to the compiler
        $<$<CONFIG:Debug>:DEBUG>
)

target_compile_definitions(sheriff_ai_perft
    PRIVATE
	    # If the debug configuration pass the DEBUG define to the compiler
        $<$<CONFIG:Debug>:DEBUG>
)

# use 
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# to switch between debug and release modes
