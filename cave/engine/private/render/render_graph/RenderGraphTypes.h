#pragma once
// @TODO: refactor
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/sampler.h"

namespace cave::render {

using ImportFunc = std::function<std::shared_ptr<GpuTexture>()>;

enum class ResourceAccess : uint8_t {
    NONE = 0,
    SRV = BIT(0),
    UAV = BIT(1),
    RTV = BIT(2),
    DSV = BIT(3),
    // Present,
    // CopySrc,
    // CopyDst,
    // DepthRead,
    // DepthWrite,
};
DEFINE_ENUM_BITWISE_OPERATIONS(ResourceAccess);

struct RGTextureNode {
    GpuTextureDesc desc{};
    SamplerDesc sampler{};
    bool is_import = false;
    ImportFunc import;

    ResourceAccess accessMask{ ResourceAccess::NONE };
};

}  // namespace cave::render
