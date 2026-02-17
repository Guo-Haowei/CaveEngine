set(TARGET_NAME chess_dll)

file(GLOB_RECURSE SRC
    "*.h"
    "*.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

add_library(${TARGET_NAME} SHARED ${SRC})

set_target_properties(${TARGET_NAME}
    PROPERTIES
    OUTPUT_NAME "game"
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/cave/public
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

set_target_properties(${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(${TARGET_NAME}
    PROPERTIES
    FOLDER games
)

# @TODO: do not link the entire engine in the game DLL in the future.
target_link_libraries(${TARGET_NAME} PRIVATE engine)

target_compile_definitions(${TARGET_NAME} PRIVATE
    CAVE_GAME_DLL=1
)

if(CAVE_BUILD_TESTS)
    add_subdirectory(core)
endif()
