// =============================================================================
// File: cave/render/rhi/Defines.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/core/memory/Pointer.h"

namespace cave::render {

struct GpuTexture;
using GpuTextureId = Ref<GpuTexture>;

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

enum class GpuBufferType : uint8_t {
    Unknown,
    Vertex,
    Index,
    Constant,
    Structured,
};

enum class PrimitiveTopology : uint8_t {
    Undefined = 0,
    Point,
    Line,
    Triangle,
    Patch,
};

enum class LoadOp : uint8_t {
    Load = 0,
    Clear,
    DontCare,
};

enum class StoreOp : uint8_t {
    Store = 0,
};

struct TextureViewDesc {
    uint16_t mip_slice = 0;          // D3D: MipSlice
    uint16_t first_array_slice = 0;  // D3D: FirstArraySlice / First2DArrayFace
    uint16_t array_size = 1;         // D3D: ArraySize / NumCubes * 6
};

struct ColorAttachmentDesc {
    GpuTextureId tex{};
    TextureViewDesc view{};
    LoadOp load{ LoadOp::Load };
    float clear_color[4]{ 0, 0, 0, 0 };
};

struct DepthAttachmentDesc {
    GpuTextureId tex{};
    TextureViewDesc view{};
    LoadOp depth_load{ LoadOp::Load };
    LoadOp stencil_load{ LoadOp::Load };
    uint8_t clear_stencil{ 0 };
    float clear_depth{ 1.0f };
};

struct RenderingInfo {
    std::span<const ColorAttachmentDesc> colors;
    std::optional<DepthAttachmentDesc> depth;
};

struct Viewport {
    Viewport(int width, int height)
        : width(width)
        , height(height)
        , top_left_x(0)
        , top_left_y(0) {}

    int width;
    int height;
    int top_left_x;
    int top_left_y;
};

}  // namespace cave::render