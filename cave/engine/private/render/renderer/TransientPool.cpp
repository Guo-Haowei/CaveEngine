#include "TransientPool.h"

#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

GpuTextureId TransientPool::acquireTexture(const TransientTextureDesc& desc) {
    // @TODO: use description hash key
    std::string key = std::format("{}@{}x{}",
                                  desc.texture.name,
                                  desc.texture.width,
                                  desc.texture.height);

    auto [it, inserted] = m_cache.try_emplace(std::move(key));
    if (!inserted) {
        return it->second;
    }

    GpuTextureId tex = m_device.createTexture(desc.texture, desc.sampler);
    it->second = tex;
    return tex;
}

#if USING(USE_RENDERER_DEBUG)
PoolSnapshot TransientPool::snapshot() const {
    PoolSnapshot snapshot;
    snapshot.textures.resize(m_cache.size());
    int idx = 0;
    for (const auto& [key, tex] : m_cache) {
        PoolTextureInfo& info = snapshot.textures[idx++];
        info.debug_name = key;
        info.width = tex->desc.width;
        info.height = tex->desc.height;
        info.depth = tex->desc.depth;
        info.bytes = info.width * info.height * info.depth;  // @TODO: bytes per pixel

        info.texture = tex;
    }
    return snapshot;
}
#endif

}  // namespace cave::render