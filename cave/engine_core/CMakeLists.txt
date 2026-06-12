set(TARGET_NAME engine_core)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${TARGET_NAME} SHARED ${SRC})

set_target_properties(${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

target_include_directories(${TARGET_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_compile_definitions(${TARGET_NAME}
    PRIVATE
        CAVE_CORE_BUILD_DLL
    PUBLIC
        _CRT_SECURE_NO_WARNINGS
        ROOT_FOLDER="${PROJECT_SOURCE_DIR}/"
)

target_set_warning_level(${TARGET_NAME})
