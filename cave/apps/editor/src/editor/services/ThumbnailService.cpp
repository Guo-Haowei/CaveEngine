#include "ThumbnailService.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/time/FrameTime.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/framework/ViewManager.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorState.h"

// @TODO: refactor
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/sampler.h"

namespace cave {

ThumbnailService::ThumbnailService(EditorState& p_editor) noexcept
    : m_view_manager(*p_editor.GetApp().GetViewManager())
    , m_scene_reg(*p_editor.GetApp().GetSceneRegistry())
    , m_render_device(*p_editor.GetApp().GetRenderDevice())
    , m_builder(p_editor.GetApp()) {
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
    rec.view_id = m_view_manager.Create();
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
            m_view_manager.Destroy(rec.view_id);
            m_scene_reg.Destroy(rec.scene_id);
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
        const PendingRequest pending = m_pending.front();
        m_pending.pop_front();

        auto it = m_cache.find(pending.key);
        if (it == m_cache.end()) continue;

        ThumbnailRecord& rec = it->second;

        // Drop stale requests
        if (rec.generation != pending.generation) {
            continue;
        }

        // Skip pending requests
        if (rec.state == ThumbnailState::Ready || rec.state == ThumbnailState::Pending) {
            continue;
        }

        PreviewBuildRequest req{
            .guid = pending.key.guid,
            .options = {
                .width = pending.key.size,
                .height = pending.key.size,
            },
        };

        PreviewBuildResult res = m_builder.Build(req);
        if (res.status != PreviewBuildStatus::Ok) {
            continue;
        }

        rec.scene_id = res.scene_id;

        // @TODO: move it to somewhere else
        GpuTextureDesc tex_desc{
            .type = AttachmentType::COLOR_2D,
            .dimension = Dimension::TEXTURE_2D,
            .width = req.options.width,
            .height = req.options.height,
            .depth = 1,
            .mipLevels = 0,
            .arraySize = 1,
            .format = PixelFormat::R16G16B16A16_FLOAT,
            .bindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE,
            .miscFlags = RESOURCE_MISC_NONE,
        };
        auto tex =
            m_render_device.CreateTexture(
                tex_desc,
                PointClampSampler());

        // submit view request
        render::ViewDesc view;
        view.view_id = rec.view_id;
        view.viewport_px = { 0, 0, (int)req.options.width, (int)req.options.height };
        view.scene_id = res.scene_id;
        view.camera_source = res.camera;
        view.output = tex;
        m_view_manager.Submit(view);

        // save record
        rec.state = ThumbnailState::Pending;
        rec.submitted_frame = m_frame_index;
        rec.texture = tex;
        rec.gpu_handle = tex->GetHandle();

        m_inflight.push_back(pending.key);
        ++submitted;

#if USING(USE_LOG)
        auto handle = AssetRegistry::GetSingleton().FindByGuid(req.guid);
        const AssetMetaData* meta = handle.unwrap().GetMeta();
        LOG_VERBOSE("ThumbnailService::SubmitRequests: '{}' job submitted", meta->name);
#endif
    }
}

void ThumbnailService::Invalidate(const Guid& p_guid) {
    unused(p_guid);
}

}  // namespace cave
