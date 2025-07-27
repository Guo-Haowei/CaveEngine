set(TARGET_NAME sw_renderer)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_executable(${TARGET_NAME} ${SRC})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/thirdparty/glm
    ${PROJECT_SOURCE_DIR}/thirdparty/
    ${PROJECT_SOURCE_DIR}/cave/
)

target_precompile_headers(${TARGET_NAME} PRIVATE pch.h)

target_link_libraries(${TARGET_NAME} PRIVATE
    engine
)

# @TODO: remove this
target_compile_definitions(${TARGET_NAME} PUBLIC
    SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
)
