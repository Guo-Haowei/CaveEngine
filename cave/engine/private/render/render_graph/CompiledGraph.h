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
    void addResource(RGTextureId handle, const Ref<GpuTexture>& resource);
    Ref<GpuTexture> FindResource(RGTextureId handle);

    std::span<const CompiledPass> GetCompiledPass() const {
        return m_compiled_pass;
    }

    void resolveTextures(TransientPool& pool);

private:
    Vector<CompiledPass> m_compiled_pass;

    Vector<Ref<GpuTexture>> m_resources;
    HashMap<RGTextureId, int> m_resource_lookup;

    // transferred from RenderGraph
    Vector<RGTextureNode> m_textures;
    Vector<RenderPass> m_passes;
    Vector<int> m_order;

    friend class RenderGraph;
};

}  // namespace cave::render
