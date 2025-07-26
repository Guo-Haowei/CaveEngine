add_subdirectory(engine)
add_subdirectory(engine_assets)
add_subdirectory(modules)
add_subdirectory(editor)
add_subdirectory(shader)

if (NOT EMSCRIPTEN)
    add_subdirectory(engine_tests)
endif()
