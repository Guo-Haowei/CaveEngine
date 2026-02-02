#pragma once
#include "RGTextureId.h"

// @TODO: refactor
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/sampler.h"

namespace cave::render {

// clang-format off
enum class ResourceAccess : uint8_t {
    NONE   = 0,
    SRV    = BIT(0),
    UAV    = BIT(1),
    RTV    = BIT(2),
    DSV    = BIT(3),
    // Present,
    // CopySrc,
    // CopyDst,
    // DepthRead,
    // DepthWrite,
};
// clang-format on
DEFINE_ENUM_BITWISE_OPERATIONS(ResourceAccess);

struct RGTextureNode {
    RGTextureId handle;
    GpuTextureDesc desc{};
    SamplerDesc sampler{};
    ResourceAccess access_mask{ ResourceAccess::NONE };
    GpuTextureId external{};

    std::string debug_name;
};

}  // namespace cave::render
