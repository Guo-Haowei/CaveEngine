#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Angle.h"
#include "cave/core/math/AABB.h"
#include "cave/render/ViewDesc.h"

#include "engine/private/render/renderer/ResolvedView.h"
#include "engine/private/render/renderer/RenderPackets.h"

#include "engine/private/core/math/color.h"
#include "engine/private/core/math/geomath.h"
#include "engine/private/renderer/debug_draw.h"
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/graphics_defines.h"

namespace cave {
#include "cbuffer.hlsl.h"
}  // namespace cave

namespace cave {

using render::DrawItem;

class Scene;

struct RenderOptions {
    bool isOpengl{ false };
    bool ssaoEnabled{ false };
    bool vxgiEnabled{ false };
    bool bloomEnabled{ false };
    bool iblEnabled{ false };
    int debugVoxelId{ 0 };
    int debugBvhDepth{ -1 };
    int voxelTextureSize{ 0 };
    float ssaoKernelRadius{ 0.0f };
};

struct PassContext {
    int pass_idx{ 0 };
};

// @TODO: refactor this
template<typename BUFFER, typename ID = ::cave::ecs::Entity>
struct BufferCache {
    std::vector<BUFFER> buffer;
    std::unordered_map<ID, uint32_t> lookup;

    uint32_t FindOrAdd(ID p_entity, const BUFFER& p_buffer) {
        auto it = lookup.find(p_entity);
        if (it != lookup.end()) {
            return it->second;
        }

        uint32_t index = static_cast<uint32_t>(buffer.size());
        lookup[p_entity] = index;
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
    // TileMap,
    // Sprite,
    Count,
};

struct FrameData {
    RenderOptions options;
    render::ResolvedView resolved_view;
    // const ViewInfo* view_info{ nullptr };

    // @TODO: multi camera & viewport

    PerFrameConstantBuffer perFrameCache;
    BufferCache<PerBatchConstantBuffer> batchCache;
    BufferCache<MaterialConstantBuffer> materialCache;
    std::vector<PerPassConstantBuffer> passCache;
    std::array<PointShadowConstantBuffer, MAX_POINT_LIGHT_SHADOW_COUNT * 6> pointShadowCache;
    BufferCache<BoneConstantBuffer> boneCache;
    // std::vector<EmitterConstantBuffer> emitterCache;

    // @TODO: rename
    std::array<std::unique_ptr<PassContext>, MAX_POINT_LIGHT_SHADOW_COUNT> pointShadowPasses;
    std::array<PassContext, 1> shadowPasses;  // @TODO: support multi ortho light

    PassContext voxelPass;
    PassContext mainPass;

    std::array<std::vector<DrawItem>, std::to_underlying(DrawPhase::Count)> commands;
    std::vector<DrawItem> tile_maps;
    std::vector<DrawItem> sprites;

    // std::vector<InstanceContext> instances;

    // std::vector<ParticleEmitterComponent> emitters;

    // @TODO: refactor
    bool bakeIbl{ false };

    struct UpdateBuffer {
        std::vector<math::Vector3f> positions;
        std::vector<math::Vector3f> normals;
        const void* id;
    };

    math::AABB voxel_gi_bound;

    DebugDraw& GetDebugDraw() { return m_debug_draw; }
    const DebugDraw& GetDebugDraw() const { return m_debug_draw; }

private:
    DebugDraw m_debug_draw;
};

}  // namespace cave
