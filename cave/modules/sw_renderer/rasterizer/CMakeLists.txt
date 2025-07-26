add_library(sw_renderer STATIC
    rasterizer.cpp
    render_target.cpp
    sampler.cpp
    JobSystem.cpp
    core_assert.cpp
    example_base.cpp
    application_win32.cpp
    main.cpp
)

target_include_directories(sw_renderer PUBLIC
    ${PROJECT_SOURCE_DIR}/thirdparty/glm
    ${PROJECT_SOURCE_DIR}/cave/
)