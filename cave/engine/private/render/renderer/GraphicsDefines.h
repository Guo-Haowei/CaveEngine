#pragma once
#include "cave/render/rhi/Defines.h"

namespace cave::render {

enum StencilFlags : uint8_t {
    STENCIL_FLAG_NONE = 0,
    STENCIL_FLAG_HIGHLIGHT = 1,
    STENCIL_FLAG_SKY = 255,
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

struct BlendDesc {
    bool alphaToCoverageEnable{ false };
    bool independentBlendEnable{ false };
    RenderTargetBlendDesc renderTargets[8]{};
};

}  // namespace cave::render
