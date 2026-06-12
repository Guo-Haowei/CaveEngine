set(TEST_NAME chess_tests)
set(UCI_NAME chess_uci)

file(GLOB_RECURSE SRC
    "*.h"
    "*.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

### test
add_executable(${TEST_NAME} ${SRC})

target_include_directories(${TEST_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/cave/engine_core/public/
    ${PROJECT_SOURCE_DIR}/cave/engine/public/
    ${PROJECT_SOURCE_DIR}/thirdparty/googletest/googletest/include
)

set_target_properties(${TEST_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(${TEST_NAME}
    PROPERTIES
    FOLDER tests
)

set(TARGET_LIBS
    gtest_main
)

target_link_libraries(${TEST_NAME} PRIVATE ${TARGET_LIBS})

target_compile_definitions(${TEST_NAME} PRIVATE
    CAVE_TEST=1
)

target_precompile_headers(${TEST_NAME} PRIVATE pch.h)

### UCI
add_executable(${UCI_NAME} ${SRC})

target_include_directories(${UCI_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/cave/engine_core/public/
    ${PROJECT_SOURCE_DIR}/cave/engine/public/
    ${PROJECT_SOURCE_DIR}/thirdparty/googletest/googletest/include
)

set_target_properties(${UCI_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(${UCI_NAME}
    PROPERTIES
    FOLDER tests
)

target_compile_definitions(${UCI_NAME} PRIVATE
    CHESS_UCI=1
)
