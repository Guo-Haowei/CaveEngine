// =============================================================================
// File: cave/rhi/Backend.h
// =============================================================================
#pragma once

namespace cave::rhi {

// clang-format off
#define BACKEND_LIST                                    \
    BACKEND_DECLARE(Null,       "Null",         null)   \
    BACKEND_DECLARE(OpenGL,     "OpenGL",       opengl) \
    BACKEND_DECLARE(Direct3D11, "Direct3D 11",  d3d11)  \
    BACKEND_DECLARE(Direct3D12, "Direct3D 12",  d3d12)  \
    BACKEND_DECLARE(Vulkan,     "Vulkan",       vulkan) \
    BACKEND_DECLARE(Metal,      "Metal",        metal)
// clang-format on

enum class Backend : uint8_t {
#define BACKEND_DECLARE(ENUM, ...) ENUM,
    BACKEND_LIST
#undef BACKEND_DECLARE
        Count,
};

}  // namespace cave::rhi