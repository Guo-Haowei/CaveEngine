set(TARGET_NAME cave_unit_tests)

file(GLOB_RECURSE SRC
    "*.h"
    "*.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

add_executable(${TARGET_NAME} ${SRC})

target_include_directories(${TARGET_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/cave/engine/
    ${PROJECT_SOURCE_DIR}/thirdparty/googletest/googletest/include
)

set_target_properties(${TARGET_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set(TARGET_LIBS
    engine
    gtest_main
)

target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_LIBS})

target_precompile_headers(${TARGET_NAME} PRIVATE pch.h)

target_set_warning_level(${TARGET_NAME})
