#pragma once
#include "cave/core/base/NonCopyable.h"

#include "CompiledPass.h"
#include "RenderGraphTypes.h"
#include "RenderPass.h"

namespace cave {

struct FrameData;

}  // namespace cave

namespace cave::render {

class TransientPool;

class CompiledGraph : public NonCopyable {
public:
    struct Edge {
        int from;
        int to;
    };

    void AddResource(RGTextureId p_handle, const std::shared_ptr<GpuTexture>& p_resource);
    std::shared_ptr<GpuTexture> FindResource(RGTextureId p_handle);

    std::span<const CompiledPass> GetCompiledPass() const { return m_compiled_pass; }

    void Resolve(TransientPool& p_pool);

private:
    std::vector<CompiledPass> m_compiled_pass;

    std::vector<std::shared_ptr<GpuTexture>> m_resources;
    std::unordered_map<RGTextureId, int> m_resourceLookup;

    // transferred from RenderGraph
    std::vector<RGTextureNode> m_textures;
    std::vector<RenderPass> m_passes;
    std::vector<int> m_order;

    friend class RenderGraph;
};

}  // namespace cave::render
