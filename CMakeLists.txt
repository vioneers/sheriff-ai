cmake_minimum_required(VERSION 3.15)
project(Sheriff-AI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)


# Generate magic.h in src/
# To force it to regenerate we have to manually remove the file.
set(MAGIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/src/magic.h")

set(ENGINE_SOURCES
    src/engine.cpp
    src/board.cpp
    src/piece.cpp
    src/move.cpp
	src/evaluationbar.cpp
	${MAGIC_HEADER}
)



# To generate ${MAGIC_HEADER} run make_magic
add_custom_command(
	OUTPUT ${MAGIC_HEADER}
    COMMAND make_magic "${GENERATED_HEADER}"
    DEPENDS make_magic
    COMMENT "Generating magic.h in src/..."
)

add_executable(make_magic
	src/make_magic.cpp
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



add_compile_definitions(
	    # If the debug configuration pass the DEBUG define to the compiler
        $<$<CONFIG:Debug>:DEBUG>
)

# use 
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
# cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
# to switch between debug and release modes
