#pragma once
#include "cave/core/ids/Entity.h"

// clang-format off
namespace cave { enum StencilFlags : uint8_t; }
namespace cave { struct GpuMesh; }
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave::render {

struct DrawItem {
    struct Range {
        uint32_t offset{};
        uint32_t count{};
    };

    const GpuMesh* mesh_data = nullptr;
    const GpuTexture* texture = nullptr;
    Range index;

    int bone_idx{ -1 };
    int mat_idx{ -1 };
    int batch_idx{ -1 };

    StencilFlags flags{ 0 };

    // @TODO: implement the following
    // PsoHandle pso{};
    // Instancing
    // InstanceRange instances{};
    // InstanceDataBinding instance_data{};
};

}  // namespace cave::render

#if 0

struct ViewPacket {
    // Output
    TextureHandle color;
    TextureHandle depth;
    RectInt viewport;
    RectInt scissor;

    // GPU allocations (must stay valid until GPU done)
    GpuBufferSlice per_view_cb;

    // Passes recorded in order
    std::vector<PassPacket> passes;
};

struct PassPacket {
    PassType type;

    // Render targets for this pass (can be same as view output or intermediates)
    TextureHandle color;
    TextureHandle depth;

    RectInt viewport;
    RectInt scissor;

    // Per-pass constants
    GpuBufferSlice per_pass_cb;

    // Draw stream for this pass
    std::vector<DrawItem> draws;
};

#endif
