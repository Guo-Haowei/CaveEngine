// =============================================================================
// File: cave/render/rhi/Defines.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/core/memory/Pointer.h"

namespace cave::render {

struct GpuResource;
using GpuTextureId = Ref<GpuResource>;

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
    Buffer = 0,
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

enum class MagFilter : uint8_t {
    Point,
    Linear,
};

enum class MinFilter : uint8_t {
    Point,
    Linear,
    LinearMipmapLinear,
};

enum class AddressMode : uint8_t {
    Wrap,
    Clamp,
    Border,
};

enum class StaticBorderColor {
    TransparentBlack = 0,
    OpaqueBlack,
    OpaqueWhite,
};

enum class InputClassification : uint8_t {
    PerVertexData = 0,
    PerInstanceData,
};

enum class FillMode : uint8_t {
    Wireframe = 0,
    Solid,
};

enum class CullMode : uint8_t {
    None = 0,
    Front,
    Back,
    FrontAndBack,
};

enum class Blend : uint8_t {
    BLEND_ZERO = 1,
    BLEND_ONE,
    BLEND_SRC_ALPHA,
    BLEND_INV_SRC_ALPHA,
};

enum class BlendOp : uint8_t {
    BLEND_OP_ADD,
    BLEND_OP_SUB,
};

// clang-format off
enum ColorWriteEnable : uint8_t {
    COLOR_WRITE_ENABLE_NONE  = 0,
    COLOR_WRITE_ENABLE_RED   = BIT(0),
    COLOR_WRITE_ENABLE_GREEN = BIT(1),
    COLOR_WRITE_ENABLE_BLUE  = BIT(2),
    COLOR_WRITE_ENABLE_ALPHA = BIT(3),
    COLOR_WRITE_ENABLE_ALL   = COLOR_WRITE_ENABLE_RED | COLOR_WRITE_ENABLE_GREEN | COLOR_WRITE_ENABLE_BLUE | COLOR_WRITE_ENABLE_ALPHA,
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(ColorWriteEnable);

struct RenderTargetBlendDesc {
    bool blendEnabled{ false };
    Blend blendSrc{ Blend::BLEND_ONE };
    Blend blendDest{ Blend::BLEND_ZERO };
    BlendOp blendOp{ BlendOp::BLEND_OP_ADD };
    ColorWriteEnable colorWriteMask{ COLOR_WRITE_ENABLE_ALL };
};

#define COMPARISON_FUNC_ENUM_LIST          \
    COMPARISON_FUNC_ENUM(NEVER, 1)         \
    COMPARISON_FUNC_ENUM(LESS, 2)          \
    COMPARISON_FUNC_ENUM(EQUAL, 3)         \
    COMPARISON_FUNC_ENUM(LESS_EQUAL, 4)    \
    COMPARISON_FUNC_ENUM(GREATER, 5)       \
    COMPARISON_FUNC_ENUM(NOT_EQUAL, 6)     \
    COMPARISON_FUNC_ENUM(GREATER_EQUAL, 7) \
    COMPARISON_FUNC_ENUM(ALWAYS, 8)

enum class ComparisonFunc : uint8_t {
#define COMPARISON_FUNC_ENUM(ENUM, VALUE) ENUM = VALUE,
    COMPARISON_FUNC_ENUM_LIST
#undef COMPARISON_FUNC_ENUM
};

#define STENCIL_OP_ENUM_LIST     \
    STENCIL_OP_ENUM(KEEP, 1)     \
    STENCIL_OP_ENUM(ZERO, 2)     \
    STENCIL_OP_ENUM(REPLACE, 3)  \
    STENCIL_OP_ENUM(INCR_SAT, 4) \
    STENCIL_OP_ENUM(DECR_SAT, 5) \
    STENCIL_OP_ENUM(INVERT, 6)   \
    STENCIL_OP_ENUM(INCR, 7)     \
    STENCIL_OP_ENUM(DECR, 8)

enum class StencilOp : uint8_t {
#define STENCIL_OP_ENUM(ENUM, VALUE) ENUM = VALUE,
    STENCIL_OP_ENUM_LIST
#undef STENCIL_OP_ENUM
};

struct BlendDesc {
    bool alphaToCoverageEnable{ false };
    bool independentBlendEnable{ false };
    RenderTargetBlendDesc renderTargets[8]{};
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

// @TODO: remove this
enum StencilFlags : uint8_t {
    STENCIL_FLAG_NONE = 0,
    STENCIL_FLAG_HIGHLIGHT = 1,
    STENCIL_FLAG_SKY = 255,
};

}  // namespace cave::render