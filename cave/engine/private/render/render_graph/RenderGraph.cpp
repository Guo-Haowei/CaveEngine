#include "RenderGraph.h"

namespace cave::render {

void RenderGraph::AddResource(RGTextureId p_handle, const std::shared_ptr<GpuTexture>& p_resource) {
    const int idx = static_cast<int>(m_resources.size());
    m_resources.push_back(p_resource);
    m_resourceLookup.insert({ p_handle, idx });
}

std::shared_ptr<GpuTexture> RenderGraph::FindResource(RGTextureId p_handle) {
    auto it = m_resourceLookup.find(p_handle);
    if (it == m_resourceLookup.end()) {
        return nullptr;
    }

    return m_resources[it->second];
}

void RenderGraph::AddPass(const std::string& p_name, const std::shared_ptr<RGRenderPass>& p_pass) {
    const int idx = static_cast<int>(m_renderPasses.size());
    m_renderPasses.push_back(p_pass);
    m_renderPassLookup.insert({ p_name, idx });
}

RGRenderPass* RenderGraph::FindPass(const std::string& p_name) {
    auto it = m_renderPassLookup.find(p_name);
    if (it == m_renderPassLookup.end()) {
        return nullptr;
    }

    return m_renderPasses[it->second].get();
}

}  // namespace cave::render
