#pragma once
#include "cave/core/ids/Entity.h"

namespace cave::render {

enum StencilFlags : uint8_t;
struct GpuMesh;
struct GpuResource;

struct DrawItem {
    struct Range {
        uint32_t offset{};
        uint32_t count{};
    };

    const GpuMesh* mesh_data = nullptr;
    const GpuResource* texture = nullptr;
    Range index;

    int bone_idx = -1;
    int mat_idx = -1;
    int batch_idx = -1;
    int z_index = 0;

    StencilFlags flags{};

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
