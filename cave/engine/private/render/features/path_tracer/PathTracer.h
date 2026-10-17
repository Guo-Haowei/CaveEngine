#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/math/Box.h"
#include "cave/render/rhi/GpuResources.h"

#include "PathTracerFeature.h"

// clang-format off
namespace cave { class Scene; }
// clang-format on

namespace cave::render {

class PathTracer {
public:
    void setMode(PathTracerMode mode) { m_mode = mode; }

    void update(const Scene& scene);

    bool isActive() const;

private:
    bool createAccelStructure(const Scene& scene);
    void updateAccelStructure(const Scene& scene);

    Ref<GpuResource> m_bvh_buffer;
    Ref<GpuResource> m_vertex_buffer;
    Ref<GpuResource> m_index_buffer;
    Ref<GpuResource> m_mesh_buffer;
    Ref<GpuResource> m_material_buffer;

    struct MeshData {
        int root_bvh_id;
    };

    // @TODO: rename
    HashMap<Guid, MeshData> m_meshs;

    PathTracerMode m_mode{ PathTracerMode::None };

    friend class PathTracerFeature;
};

}  // namespace cave::render
