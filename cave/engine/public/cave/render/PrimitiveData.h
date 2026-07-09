// =============================================================================
// File: cave/render/PrimitiveData.h
// =============================================================================
#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/core/math/Vec.h"

// clang-format off
namespace cave { struct GpuTexture; }
// clang-format on

namespace cave::render {

struct PrimVert {
    math::Vec3f pos;
    math::Vec2f uv;
    math::Vec4f color;
};

struct PrimBatch {
    uint32_t idx_offset = 0;
    uint32_t idx_count = 0;
    GpuTexture* tex{};
};

struct PrimData {
    Vector<PrimVert> vertices;
    Vector<uint32_t> indices;
    Vector<PrimBatch> cmds;
};

}  // namespace cave::render
