set(TARGET_NAME super_cave_boy)

file(GLOB_RECURSE SRC
    "*.h"
    "*.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

add_library(${TARGET_NAME} SHARED ${SRC})

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/cave/public
    ${CMAKE_CURRENT_SOURCE_DIR}/
)

# @TODO: do not link the entire engine in the game DLL in the future.
target_link_libraries(${TARGET_NAME} PRIVATE engine)

set_target_properties(${TARGET_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    FOLDER games
)

target_compile_definitions(${TARGET_NAME} PRIVATE
    CAVE_GAME_DLL=1
)

target_set_warning_level(${TARGET_NAME})
target_set_debug_symbol(${TARGET_NAME})
