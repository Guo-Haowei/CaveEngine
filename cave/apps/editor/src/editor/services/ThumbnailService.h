#pragma once
#include "engine/private/renderer/gpu_resource.h"

#include "editor/thumbnail/ThumbnailKey.h"
#include "editor/thumbnail/PreviewBuilder.h"

// clang-format off
namespace cave::render { class IRenderDevice; }
// clang-format on

namespace cave {

struct EngineServices;
struct FrameTime;
class SceneRegistry;
class ViewManager;

enum class ThumbnailState : uint8_t {
    Missing = 0,
    Pending,
    Ready,
    Failed,
};

struct BusyInfo {
    uint32_t normal_view_count = 0;
    bool is_interacting = false;
};

struct ThumbnailRecord {
    ViewDesc view_desc;
    ThumbnailState state{};

    uint64_t gpu_handle{};
    uint64_t last_used_frame{};
    uint64_t submitted_frame{};
    uint32_t generation{};
};

class ThumbnailService {
    struct PendingRequest {
        ThumbnailKey key{};
        uint32_t generation = 0;
    };

public:
    explicit ThumbnailService(EngineServices& services) noexcept;

    uint64_t getOrRequest(const ThumbnailKey& key);

    void tick(const FrameTime& time, const BusyInfo& info);

    void invalidate(const Guid& guid);

private:
    void processCompletions();
    void submitRequests(const BusyInfo& info);

    ViewManager& view_manager_;
    SceneRegistry& scene_reg_;
    render::IRenderDevice& render_device_;
    PreviewBuilder builder_;

    uint64_t frame_index_{};

    std::list<PendingRequest> pending_;
    std::list<ThumbnailKey> inflight_;
    std::unordered_map<ThumbnailKey, ThumbnailRecord> cache_;
};

}  // namespace cave
