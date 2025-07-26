include(FetchContent)

FetchContent_Declare(
    yaml-cpp
    GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
    GIT_TAG        0.8.0
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
    LuaBridge3
    GIT_REPOSITORY https://github.com/kunitoki/LuaBridge3.git
    GIT_TAG        3.0-rc3
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(LuaBridge3)

set_target_properties(yaml-cpp PROPERTIES FOLDER thirdparty)
set_target_properties(yaml-cpp-parse PROPERTIES FOLDER thirdparty)
set_target_properties(yaml-cpp-read PROPERTIES FOLDER thirdparty)
set_target_properties(yaml-cpp-sandbox PROPERTIES FOLDER thirdparty)

set(TARGET_NAME engine)

file(GLOB_RECURSE SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.inl"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.natvis"
)
set(FILTERED_SRC "")

foreach(file ${SRC})
    # ignore generated files
    string(REGEX MATCH "\\.generated\\.cpp$" is_generated ${file})
    if(NOT is_generated)
        if(WIN32)
            if(file MATCHES "apple" OR file MATCHES "metal" )
                # filtered
            else()
                list(APPEND FILTERED_SRC ${file})
            endif()
        elseif(APPLE)
            if(file MATCHES "d3d" OR file MATCHES "windows" OR file MATCHES "vk")
                # filtered
            else()
                list(APPEND FILTERED_SRC ${file})
            endif()
        elseif(EMSCRIPTEN)
            if(file MATCHES "d3d" OR file MATCHES "windows" OR file MATCHES "vk" OR file MATCHES "metal")
                # filtered
            else()
                list(APPEND FILTERED_SRC ${file})
            endif()
        else()
            message(FATAL_ERROR "Platform not supported")
        endif()
    endif()
endforeach()

add_library(${TARGET_NAME} STATIC ${FILTERED_SRC})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/ FILES ${FILTERED_SRC})

target_include_directories(${TARGET_NAME} PUBLIC
    ${luabridge3_SOURCE_DIR}/Source
    ${PROJECT_SOURCE_DIR}/thirdparty/lua
    ${PROJECT_SOURCE_DIR}/thirdparty/imgui
    ${PROJECT_SOURCE_DIR}/thirdparty/optick
    ${PROJECT_SOURCE_DIR}/thirdparty/glm
    ${PROJECT_SOURCE_DIR}/thirdparty/
    ${PROJECT_SOURCE_DIR}/cave/shader
    ${PROJECT_SOURCE_DIR}/cave/
    ${PROJECT_SOURCE_DIR}/plugins/src
    ${PROJECT_SOURCE_DIR}/
)

target_compile_definitions(${TARGET_NAME} PUBLIC
    -D_CRT_SECURE_NO_WARNINGS
    -DROOT_FOLDER="${PROJECT_SOURCE_DIR}/"
)

set(TARGET_LIBS
    engine_assets
    imgui
    glm::glm
    lua
    yaml-cpp
)

if(WIN32)
    list(APPEND TARGET_LIBS
        glfw
        optick
        loader_assimp
        opengl_4_backend
        d3d11_backend
        d3d12_backend
        vk_backend
    )
elseif (EMSCRIPTEN)
    list(APPEND TARGET_LIBS
        opengles_3_backend
    )
endif()

target_link_libraries(${TARGET_NAME} PUBLIC ${TARGET_LIBS})

target_set_warning_level(${TARGET_NAME})

target_precompile_headers(${TARGET_NAME} PRIVATE pch.h)

if(APPLE)
    target_link_libraries(${TARGET_NAME} PRIVATE
        "-framework Metal"
        "-framework MetalKit"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework QuartzCore"
    )
endif()