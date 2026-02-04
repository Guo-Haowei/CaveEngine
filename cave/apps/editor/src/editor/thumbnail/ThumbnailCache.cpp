#include "ThumbnailCache.h"

namespace cave {

uint64_t ThumbnailCache::TryGetHandle(const ThumbnailKey& p_key) noexcept {
    unused(p_key);
    return 0;
}

ThumbnailRecord& ThumbnailCache::GetOrCreate(const ThumbnailKey& p_key) noexcept {
    return m_cache[p_key];
}

}  // namespace cave
