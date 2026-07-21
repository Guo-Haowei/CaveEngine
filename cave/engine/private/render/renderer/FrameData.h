#pragma once
#include "cave/core/Color.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Angle.h"
#include "cave/core/math/AABB.h"
#include "cave/runtime/view/ViewDesc.h"

#include "RenderPackets.h"

#include "engine/private/core/math/geomath.h"
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/graphics_defines.h"
#include "engine/private/runtime/view/ResolvedView.h"

namespace cave {
#include "cbuffer.hlsl.h"
}  // namespace cave

namespace cave::render {

struct RenderOptions {
    bool is_opengl{ false };
    bool enable_ssao{ false };
    bool enable_bloom{ false };
    bool enable_ibl{ false };

    // @TODO: refactor the following
    bool vxgiEnabled{ false };
    int debugVoxelId{ 0 };
    int debugBvhDepth{ -1 };
    int voxelTextureSize{ 0 };
    float ssaoKernelRadius{ 0.0f };
};

}  // namespace cave::render

namespace cave {

using render::DrawItem;

class Scene;

struct PassContext {
    int pass_idx{ 0 };
};

// @TODO: refactor this
template<typename BUFFER, typename ID = ::cave::ecs::Entity>
struct BufferCache {
    Vector<BUFFER> buffer;
    HashMap<ID, uint32_t> lookup;

    uint32_t FindOrAdd(ID p_ent, const BUFFER& p_buffer) {
        auto it = lookup.find(p_ent);
        if (it != lookup.end()) {
            return it->second;
        }

        uint32_t index = static_cast<uint32_t>(buffer.size());
        lookup[p_ent] = index;
        buffer.emplace_back(p_buffer);
        return index;
    }

    void Clear() {
        buffer.clear();
        lookup.clear();
    }
};

enum class DrawPhase : uint8_t {
    Shadow = 0,
    DepthPrepass,
    Deferred,
    Forward,
    Voxelization,
    Count,
};

struct FrameData {
    ViewId view_id;
    render::RenderOptions options;
    // @TODO: multi camera & viewport

    PerFrameConstantBuffer perFrameCache;
    BufferCache<PerBatchConstantBuffer> batchCache;
    BufferCache<MaterialConstantBuffer> materialCache;
    Vector<PerPassConstantBuffer> passCache;
    std::array<PointShadowConstantBuffer, MAX_POINT_LIGHT_SHADOW_COUNT * 6> pointShadowCache;
    BufferCache<BoneConstantBuffer> boneCache;
    // std::vector<EmitterConstantBuffer> emitterCache;

    // @TODO: rename
    std::array<Owner<PassContext>, MAX_POINT_LIGHT_SHADOW_COUNT> pointShadowPasses;
    std::array<PassContext, 1> shadowPasses;  // @TODO: support multi ortho light

    PassContext voxelPass;
    PassContext mainPass;

    std::array<Vector<DrawItem>, std::to_underlying(DrawPhase::Count)> commands;

    // std::vector<InstanceContext> instances;

    // std::vector<ParticleEmitterComponent> emitters;

    struct UpdateBuffer {
        Vector<math::Vec3f> positions;
        Vector<math::Vec3f> normals;
        const void* id;
    };

    math::AABB voxel_gi_bound;
};

}  // namespace cave
