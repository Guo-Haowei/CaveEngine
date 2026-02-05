#pragma once
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/sampler.h"

#include "RendererDebug.h"

namespace cave::render {

class IRenderDevice;

// just use unique name for reusing
struct TransientTextureDesc {
    GpuTextureDesc texture;
    SamplerDesc sampler;
};

class TransientPool {
public:
    TransientPool(IRenderDevice& p_device)
        : m_device(p_device) {}

    GpuTextureId AcquireTexture(const TransientTextureDesc& p_desc);

#if USING(USE_RENDERER_DEBUG)
    PoolSnapshot Snapshot() const;
#endif

private:
    IRenderDevice& m_device;
    std::unordered_map<std::string, GpuTextureId> m_cache;
};

}  // namespace cave::render