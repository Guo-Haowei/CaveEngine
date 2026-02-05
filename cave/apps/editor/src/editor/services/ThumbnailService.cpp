#include "ThumbnailService.h"

#include "cave/core/time/FrameTime.h"

#include "cave/core/diagnostics/Profiler.h"

namespace cave {

ThumbnailService::ThumbnailService(EditorState& p_editor) noexcept
    : m_editor(p_editor) {
}

uint64_t ThumbnailService::GetOrRequest(const ThumbnailKey& p_key) {
    auto [it, inserted] = m_cache.try_emplace(p_key);
    if (!inserted) {
        ThumbnailRecord& rec = it->second;
        rec.last_used_frame = m_frame_index;
        if (rec.state == ThumbnailState::Ready) {
            return rec.gpu_handle;
        }
        return 0;
    }

    ThumbnailRecord& rec = it->second;
    rec.state = ThumbnailState::Missing;
    rec.last_used_frame = m_frame_index;
    rec.generation = 1;

    m_pending.emplace_back(PendingRequest{ p_key, rec.generation });
    return 0;
}

void ThumbnailService::Tick(const FrameTime& p_time, const BusyInfo& p_info) {
    ProcessCompletions();
    m_frame_index = p_time.frame_index;
    SubmitRequests(p_info);
}

static int ComputeBudget(const BusyInfo& p_info) {
    unused(p_info);
    return 1;
}

void ThumbnailService::ProcessCompletions() {
    for (const ThumbnailKey& key : m_inflight) {
        auto it = m_cache.find(key);
        if (it == m_cache.end()) continue;

        ThumbnailRecord& rec = it->second;
        if (rec.state != ThumbnailState::Pending) continue;

        // if (rec.submitted_frame <= completed_frame_index)
        {
            rec.state = ThumbnailState::Ready;
        }
    }

    m_inflight.clear();
}

void ThumbnailService::SubmitRequests(const BusyInfo& p_info) {
    CAVE_PROFILE_EVENT();

    const int budget = ComputeBudget(p_info);
    if (budget <= 0 || m_pending.empty()) {
        return;
    }

    int submitted = 0;
    while (!m_pending.empty() && submitted < budget) {
        const PendingRequest req = m_pending.front();
        m_pending.pop_front();

        auto it = m_cache.find(req.key);
        if (it == m_cache.end()) continue;

        ThumbnailRecord& rec = it->second;

        // Drop stale requests
        if (rec.generation != req.generation) {
            continue;
        }

        // Skip pending requests
        if (rec.state == ThumbnailState::Ready || rec.state == ThumbnailState::Pending) {
            continue;
        }

        // 1) Setup preview scene and camera
        // 2) Prepare render target
        // 3) Send view to view manager
#if 0
        // Record state
#endif
        rec.state = ThumbnailState::Pending;
        rec.submitted_frame = m_frame_index;
        // rec.gpu_handle = color_handle;

        m_inflight.push_back(req.key);
        ++submitted;
    }
}

void ThumbnailService::Invalidate(const Guid& p_guid) {
    unused(p_guid);
}

}  // namespace cave
