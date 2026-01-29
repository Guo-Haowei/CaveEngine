#include "RenderSystem.h"

#include "engine/private/render/renderer/RenderSystemImpl.h"

namespace cave {

// render systems
extern void RunMeshRenderSystem(Scene* p_scene, FrameData& p_framedata);
extern void RunTileMapRenderSystem(Scene* p_scene, FrameData& p_framedata);

extern void RunSpriteRenderSystem(const Scene* p_scene, FrameData& p_framedata);
extern void RunDebugRenderSystem(const Scene* p_scene, FrameData& p_framedata);

RenderSystem::RenderSystem()
    : Module("RenderSystem") {}

RenderSystem::~RenderSystem() = default;

auto RenderSystem::InitializeImpl() -> Result<void> {
    m_impl = std::make_unique<render::RenderSystemImpl>(*m_app);
    return Result<void>();
}

void RenderSystem::FinalizeImpl() {
    m_impl.reset();
}

void RenderSystem::BeginFrame() {
    m_impl->BeginFrame();
}

void RenderSystem::RenderFrame(const std::vector<render::ViewDesc>& p_views) {
    m_impl->RenderFrame(p_views);
}

const FrameData* RenderSystem::GetFrameData() const {
    return m_impl->GetFrameData();
}

}  // namespace cave
