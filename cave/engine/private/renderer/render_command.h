#pragma once

// clang-format off
namespace cave { enum StencilFlags : uint8_t; }
namespace cave { struct GpuMesh; }
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave {

enum class RenderCommandType {
    Draw,
    Compute,
};

struct DrawCommand {
    uint32_t index_count = 0;
    uint32_t index_offset = 0;

    uint32_t instance_count = 1;
    uint32_t instance_offset = 0;

    const GpuMesh* mesh_data = nullptr;
    const GpuTexture* texture = nullptr;

    int bone_idx = -1;
    int mat_idx = -1;
    int batch_idx = -1;

    uint8_t sortKey = 0;
    StencilFlags flags{ 0 };
};

struct ComputeCommand {
    int dispatchSize[3];
};

struct RenderCommand {
    RenderCommandType type;

    union {
        DrawCommand draw;
        ComputeCommand compute;
    };

    static RenderCommand From(const DrawCommand& p_draw) {
        return { RenderCommandType::Draw, { .draw = p_draw } };
    }

    static RenderCommand From(const ComputeCommand& p_compute) {
        return { RenderCommandType::Compute, { .compute = p_compute } };
    }
};

}  // namespace cave