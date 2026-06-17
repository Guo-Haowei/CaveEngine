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

    uint64_t GetOrRequest(const ThumbnailKey& p_key);

    void Tick(const FrameTime& p_time, const BusyInfo& p_info);

    void Invalidate(const Guid& p_guid);

private:
    void ProcessCompletions();
    void SubmitRequests(const BusyInfo& p_info);

    ViewManager& view_manager_;
    SceneRegistry& m_scene_reg;
    render::IRenderDevice& m_render_device;
    PreviewBuilder m_builder;

    uint64_t m_frame_index{};

    std::list<PendingRequest> m_pending;
    std::list<ThumbnailKey> m_inflight;
    std::unordered_map<ThumbnailKey, ThumbnailRecord> m_cache;
};

}  // namespace cave
