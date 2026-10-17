#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/render/rhi/GpuResources.h"

// @TODO: refactor this
#include "engine/private/render/render_graph/RGTextureId.h"

namespace cave {
class Scene;
}  // namespace cave

namespace cave::render {

struct RenderOptions;
class IRenderDevice;
class RenderGraph;

enum class PathTracerMode {
    None,
    Interactive,
};

class PathTracerFeature : public NonCopyable {
public:
    struct Inputs {
        RGTextureId skybox{};
        GpuTextureId out{};
    };

    PathTracerFeature(IRenderDevice& device)
        : m_device(device) {}

    void build(RenderGraph& graph,
               const Inputs& inputs,
               const RenderOptions& plan);

    void update(Scene& scene);

    bool isActive() const;
    void setMode(PathTracerMode mode);

private:
    bool createAccelStructure(const Scene& scene);
    void updateAccelStructure(const Scene& scene);

    IRenderDevice& m_device;

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
};

}  // namespace cave::render
