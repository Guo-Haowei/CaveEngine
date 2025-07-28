set(TARGET_NAME sw_renderer)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${TARGET_NAME} STATIC ${SRC})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/thirdparty/glm
    ${PROJECT_SOURCE_DIR}/thirdparty/
    ${PROJECT_SOURCE_DIR}/cave/
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER modules)

target_set_warning_level(${TARGET_NAME})

target_precompile_headers(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/cave/engine/pch.h)
