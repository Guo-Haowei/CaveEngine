#pragma once
#include "BvhAccel.h"
#include "PathTracerFeature.h"

#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Box.h"

#include "engine/private/render/rhi/gpu_resource.h"

// @TODO: should not view raw scene
namespace cave {
class Scene;
}

namespace cave::render {

struct GpuScene {
    // @TODO: material
    Vector<GpuPtBvh> bvhs;
    Vector<GpuPtVertex> vertices;
    Vector<GpuPtIndex> indices;
};

// @TODO: make it a layer?
class PathTracer {
public:
    void setMode(PathTracerMode mode) { m_mode = mode; }

    void update(const Scene& scene);

    bool isActive() const;

    void bindData(ICommandList& cmd);

    void unbindData(ICommandList& cmd);

private:
    bool createAccelStructure(const Scene& scene);
    void updateAccelStructure(const Scene& scene);

    Ref<GpuStructuredBuffer> m_bvh_buffer;
    Ref<GpuStructuredBuffer> m_vertex_buffer;
    Ref<GpuStructuredBuffer> m_index_buffer;
    Ref<GpuStructuredBuffer> m_mesh_buffer;
    Ref<GpuStructuredBuffer> m_material_buffer;

    struct MeshData {
        int root_bvh_id;
        ecs::Entity material_id;
    };

    // @TODO: rename
    HashMap<Guid, MeshData> m_meshs;

    PathTracerMode m_mode{ PathTracerMode::NONE };
};

}  // namespace cave::render
