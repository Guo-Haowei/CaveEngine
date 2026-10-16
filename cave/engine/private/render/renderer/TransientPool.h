#pragma once
#include "cave/render/rhi/Sampler.h"

#include "RendererDebug.h"

#include "engine/private/render/rhi/gpu_resource.h"

namespace cave::render {

class IRenderDevice;

// just use unique name for reusing
struct TransientTextureDesc {
    GpuTextureDesc texture;
    SamplerDesc sampler;
};

class TransientPool {
public:
    TransientPool(IRenderDevice& device)
        : m_device(device) {}

    GpuTextureId acquireTexture(const TransientTextureDesc& desc);

#if USING(USE_RENDERER_DEBUG)
    PoolSnapshot snapshot() const;
#endif

private:
    IRenderDevice& m_device;
    HashMap<std::string, GpuTextureId> m_cache;
};

}  // namespace cave::render