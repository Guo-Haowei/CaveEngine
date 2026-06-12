set(TARGET_NAME editor)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

if(APPLE)
    add_executable(${TARGET_NAME} MACOSX_BUNDLE ${SRC})
else()
    add_executable(${TARGET_NAME} ${SRC})
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src/editor FILES ${SRC})

target_include_directories(${TARGET_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(TARGET_LIBS
    engine
    imnodes
    box2d_physics_manager
    bullet3_physics_manager
)

target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_LIBS})

set_target_properties(${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# set_target_properties(${TARGET_NAME} PROPERTIES FOLDER tools)

target_precompile_headers(${TARGET_NAME} PRIVATE src/editor/pch.h)
target_set_warning_level(${TARGET_NAME})

# @TODO: refactor
function(copy_runtime_dll target dll_target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:${dll_target}>
            $<TARGET_FILE_DIR:${target}>
    )
endfunction()

copy_runtime_dll(editor engine_core)
# copy_runtime_dll(chess_game engine_core)