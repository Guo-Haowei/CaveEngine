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