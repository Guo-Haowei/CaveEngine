// =============================================================================
// File: cave/render/rhi/Defines.h
// =============================================================================
#pragma once
#include <cstdint>

namespace cave::render {

// clang-format off
#define BACKEND_LIST                                    \
    BACKEND_DECLARE(Null,       "Null",         null)   \
    BACKEND_DECLARE(OpenGl,     "OpenGL",       opengl) \
    BACKEND_DECLARE(D3d11,      "Direct3D 11",  d3d11)  \
    BACKEND_DECLARE(D3d12,      "Direct3D 12",  d3d12)  \
    BACKEND_DECLARE(Vulkan,     "Vulkan",       vulkan) \
    BACKEND_DECLARE(Metal,      "Metal",        metal)
// clang-format on

enum class Backend : uint8_t {
#define BACKEND_DECLARE(ENUM, ...) ENUM,
    BACKEND_LIST
#undef BACKEND_DECLARE
        Count,
};

enum class Dimension : uint8_t {
    Texture1D,
    Texture1DArray,
    Texture2D,
    Texture2DArray,
    TextureCube,
    TextureCubeArray,
    Texture3D,
};

enum class BufferUsage : uint8_t {
    Default = 0,
    Immutable = 1,
    Dynamic = 2,
    Staging = 3,
};

}  // namespace cave::render