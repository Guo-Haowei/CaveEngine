add_subdirectory(sdk)
add_subdirectory(engine)
add_subdirectory(engine_assets)
add_subdirectory(modules)
add_subdirectory(shader)

# todo: move editor to apps/
add_subdirectory(apps)

if(NOT EMSCRIPTEN)
    add_subdirectory(tools)
endif()