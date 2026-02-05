#include "TransientPool.h"

#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

GpuTextureId TransientPool::AcquireTexture(const TransientTextureDesc& p_desc) {
    // @TODO: use description hash key
    std::string key = std::format("{}@{}x{}",
                                  p_desc.texture.name,
                                  p_desc.texture.width,
                                  p_desc.texture.height);

    auto [it, inserted] = m_cache.try_emplace(std::move(key));
    if (!inserted) {
        return it->second;
    }

    // LOG_WARN("cache miss for {}", key);
    GpuTextureId tex = m_device.CreateTexture(p_desc.texture, p_desc.sampler);
    it->second = tex;
    return tex;
}

#if USING(USE_RENDERER_DEBUG)
PoolSnapshot TransientPool::Snapshot() const {
    PoolSnapshot snapshot;
    snapshot.textures.resize(m_cache.size());
    int idx = 0;
    for (const auto& [key, tex] : m_cache) {
        PoolTextureInfo& info = snapshot.textures[idx++];
        info.debug_name = key;
        info.width = tex->desc.width;
        info.height = tex->desc.height;
        info.depth = tex->desc.depth;
    }
    return snapshot;
}
#endif

}  // namespace cave::render