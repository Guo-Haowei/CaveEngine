add_executable(02-texture texture_test.cpp)

target_link_libraries(02-texture PRIVATE
    sw_application
    sw_common
    sw_renderer
)
