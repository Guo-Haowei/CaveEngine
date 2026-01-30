#include "RenderSystem.h"

#include "engine/private/render/renderer/RenderSystemImpl.h"

namespace cave {

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

void RenderSystem::RenderFrame(std::span<const render::ViewDesc> p_views) {
    m_impl->RenderFrame(p_views);
}

std::span<const FrameData> RenderSystem::GetFrameData() const {
    return m_impl->GetFrameData();
}

}  // namespace cave
