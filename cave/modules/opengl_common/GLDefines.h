#pragma once

#if USING(PLATFORM_WINDOWS)
#define CAVE_GL_VERSION_MAJOR    4
#define CAVE_GL_VERSION_MINOR    1
#define CAVE_GLSL_VERSION_STRING "410"
#elif USING(PLATFORM_APPLE)
#define CAVE_GL_VERSION_MAJOR    4
#define CAVE_GL_VERSION_MINOR    1
#define CAVE_GLSL_VERSION_STRING "410"
#else
#error "NOT IMPLEMENTED"
#endif

namespace cave::gl {

enum BUFFER_TYPE : uint32_t;
enum COMPARISON_FUNC : uint32_t;
enum STENCIL_OP : uint32_t;
enum BLEND : uint32_t;
enum BLEND_OP : uint32_t;
enum TOPOLOGY : uint32_t;

}  // namespace cave::gl
