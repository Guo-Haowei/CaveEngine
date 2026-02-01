#pragma once
#include "cave/core/NonCopyable.h"

#include "RenderGraphTypes.h"
#include "RGRenderPass.h"

namespace cave {

struct FrameData;

}  // namespace cave

namespace cave::render {

class RenderDevice;

class RenderGraph : public NonCopyable {
public:
    struct Edge {
        int from;
        int to;
    };

    void AddResource(RGTextureId p_handle, const std::shared_ptr<GpuTexture>& p_resource);
    std::shared_ptr<GpuTexture> FindResource(RGTextureId p_handle);

    void AddPass(const std::string& p_name, const std::shared_ptr<RGRenderPass>& p_pass);
    RGRenderPass* FindPass(const std::string& p_name);

    const auto& GetRenderPasses() const { return m_renderPasses; }

private:
    std::vector<std::shared_ptr<RGRenderPass>> m_renderPasses;
    std::unordered_map<std::string, int> m_renderPassLookup;

    std::vector<std::shared_ptr<GpuTexture>> m_resources;
    std::unordered_map<RGTextureId, int> m_resourceLookup;

    friend class RenderGraphBuilder;
    friend class RenderDevice;
};

}  // namespace cave::render
