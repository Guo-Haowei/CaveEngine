#pragma once
#include "cave/core/base/NonCopyable.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"
#include "cave/render/rhi/GpuResources.h"

// @TODO: refactor this
#include "engine/private/render/render_graph/RGTextureId.h"
#include "engine/private/runtime/framework/EventQueue.h"

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

class PathTracerFeature : public NonCopyable,
                          public EventListener {
public:
    struct Inputs {
        RGTextureId skybox{};
        GpuTextureId out{};
    };

    PathTracerFeature(EventQueue& event_queue, IRenderDevice& device);
    ~PathTracerFeature();

    void build(RenderGraph& graph,
               const Inputs& inputs,
               const RenderOptions& plan);

    void update(Scene& scene);

    bool isActive() const;
    void setMode(PathTracerMode mode);

    bool consumeReset();

    void eventReceived(const IEvent* event) override;

private:
    bool createAccelStructure(const Scene& scene);
    void updateAccelStructure(const Scene& scene);

    EventQueue& m_event_queue;
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

    bool m_reset_pending = true;
};

}  // namespace cave::render
