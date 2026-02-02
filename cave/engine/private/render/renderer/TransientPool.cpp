#include "TransientPool.h"

#include "engine/private/runtime/framework/IRenderDevice.h"

namespace cave::render {

GpuTextureId TransientPool::AcquireTexture(const TransientTextureDesc& p_desc) {
    const std::string& key = p_desc.texture.name;
    DEV_ASSERT(!key.empty());
    auto [it, inserted] = m_cache.try_emplace(key);
    if (!inserted) {
        return it->second;
    }

    // LOG_WARN("cache miss for {}", key);
    GpuTextureId tex = m_device.CreateTexture(p_desc.texture, p_desc.sampler);
    it->second = tex;
    return tex;
}

GpuTextureId TransientPool::TryGetTexture(const std::string& p_key) {
    if (auto it = m_cache.find(p_key); it != m_cache.end()) {
        return it->second;
    }

    return nullptr;
}

}  // namespace cave::render