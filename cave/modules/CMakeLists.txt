# @TODO: move backend to RHI
add_subdirectory(opengl_common)
if(WIN32)
    add_subdirectory(opengl4)
    add_subdirectory(d3d_common)
    add_subdirectory(d3d11)
    add_subdirectory(d3d12)
    add_subdirectory(vk)
elseif(EMSCRIPTEN)
    add_subdirectory(opengles3)
endif()

add_subdirectory(sw)
# TODO: move above modules to RHI

add_subdirectory(tinygltf)
if(CAVE_BUILD_ASSIMP)
    add_subdirectory(assimp)
endif()

if(NOT EMSCRIPTEN)
    add_subdirectory(box2d)
    add_subdirectory(bullet3)
endif()
