#pragma once
#include "engine/private/renderer/gpu_resource.h"

#include "editor/thumbnail/ThumbnailKey.h"

namespace cave {

enum class ThumbnailState : uint8_t {
    Missing = 0,
    Pending,
    Ready,
    Failed,
};

struct ThumbnailRecord {
    ThumbnailState state = ThumbnailState::Missing;
    GpuTextureId texture = {};
    uint64_t gpu_handle = 0;
    uint64_t last_used_frame = 0;
    uint64_t submitted_frame = 0;
    uint64_t version = 0;
};

class ThumbnailCache {
public:
    uint64_t TryGetHandle(const ThumbnailKey& p_key) noexcept;

    ThumbnailRecord& GetOrCreate(const ThumbnailKey& p_key) noexcept;

private:
    std::unordered_map<ThumbnailKey, ThumbnailRecord> m_cache;
};

}  // namespace cave