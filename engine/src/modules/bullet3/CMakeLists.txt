include(FetchContent)

FetchContent_Declare(
  bullet
  GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
  GIT_TAG        master  # Or a stable release like 3.25
)

set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(INSTALL_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "" FORCE)
set(ENABLE_VHACD OFF CACHE BOOL "" FORCE)
set(USE_GLUT OFF CACHE BOOL "" FORCE)
set(BUILD_ENET OFF CACHE BOOL "" FORCE)
set(BUILD_CLSOCKET OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(bullet)

set(BULLET_TARGETS
    Bullet2FileLoader
    Bullet3Collision
    Bullet3Common
    Bullet3Dynamics
    Bullet3Geometry
    Bullet3OpenCL_clew

    BulletCollision
    BulletDynamics
    BulletInverseDynamics
    BulletSoftBody
    LinearMath
)

# Group under "thirdparty/bullet"
foreach(target IN LISTS BULLET_TARGETS)
    if (TARGET ${target})
        set_target_properties(${target} PROPERTIES FOLDER thirdparty/bullet)
    endif()
endforeach()

set(TARGET_NAME bullet3_physics_manager)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

add_library(${TARGET_NAME} STATIC ${SRC})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SRC})

target_include_directories(${TARGET_NAME} PRIVATE
    ${bullet_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/thirdparty/glm
    ${PROJECT_SOURCE_DIR}/thirdparty/optick
    ${PROJECT_SOURCE_DIR}/engine/src
    ${PROJECT_SOURCE_DIR}/engine/shader
)

target_link_libraries(${TARGET_NAME} PRIVATE
    BulletDynamics
    BulletCollision
    LinearMath
    BulletSoftBody
)

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER modules)

target_set_warning_level(${TARGET_NAME})

target_precompile_headers(${TARGET_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/engine/src/engine/pch.h)
