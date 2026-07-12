#include "ThumbnailService.h"

#include "cave/core/diagnostics/Profiler.h"
#include "cave/core/time/FrameTime.h"
#include "cave/runtime/framework/EngineServices.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

// @TODO: refactor
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/sampler.h"

namespace cave {

ThumbnailService::ThumbnailService(EngineServices& services) noexcept
    : m_view_manager(services.viewManager())
    , m_scene_reg(services.sceneRegistry())
    , m_render_device(services.renderDevice())
    , m_preview_builder(services) {
}

uint64_t ThumbnailService::getOrRequest(const ThumbnailKey& key) {
    auto [it, inserted] = m_thumbnail_cache.try_emplace(key);
    if (!inserted) {
        ThumbnailRecord& rec = it->second;
        rec.last_used_frame = m_frame_index;
        if (rec.state == ThumbnailState::Ready) {
            return rec.gpu_handle;
        }
        return 0;
    }

    const uint32_t w = key.size;
    const uint32_t h = key.size;

    PreviewBuildRequest req = {
        .guid = key.guid,
        .options = {
            .width = w,
            .height = h,
        },
    };

    PreviewBuildResult res = m_preview_builder.build(req);
    if (res.status != PreviewBuildStatus::Ok) {
        return 0;
    }

    GpuTextureDesc tex_desc{
        .type = AttachmentType::COLOR_2D,
        .dimension = Dimension::TEXTURE_2D,
        .width = w,
        .height = h,
        .depth = 1,
        .mipLevels = 0,
        .arraySize = 1,
        .format = PixelFormat::R16G16B16A16_FLOAT,
        .bindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE,
        .miscFlags = RESOURCE_MISC_NONE,
    };
    auto tex = m_render_device.CreateTexture(
        tex_desc,
        PointClampSampler());

    ThumbnailRecord& rec = it->second;
    math::IntRect vp = { 0, 0, (int)w, (int)h };
    rec = {
        .view_desc = {
            .view_id = m_view_manager.createView("ThumbnailView", vp),
            .scene_id = res.scene_id,
            .camera_source = res.camera,
            .viewport_px = vp,
            .highlight = {},
            .output = tex,
        },
        .state = ThumbnailState::Missing,
        .gpu_handle = tex->GetHandle(),
        .last_used_frame = m_frame_index,
        .submitted_frame = 0,
        .generation = 1,
    };

    m_pending.emplace_back(PendingRequest{ key, rec.generation });
    return 0;
}

void ThumbnailService::tick(const FrameTime& time, const BusyInfo& info) {
    processCompletions();
    m_frame_index = time.frame_index;
    submitRequests(info);
}

static int ComputeBudget(const BusyInfo& info) {
    unused(info);
    return 1;
}

void ThumbnailService::processCompletions() {
    for (const ThumbnailKey& key : m_inflight) {
        auto it = m_thumbnail_cache.find(key);
        if (it == m_thumbnail_cache.end()) continue;

        ThumbnailRecord& rec = it->second;
        if (rec.state != ThumbnailState::Pending) continue;

        // if (rec.submitted_frame <= completed_frame_index)
        {
            rec.state = ThumbnailState::Ready;
            m_view_manager.destroyView(rec.view_desc.view_id);
            m_scene_reg.destroyScene(rec.view_desc.scene_id);
        }
    }

    m_inflight.clear();
}

void ThumbnailService::submitRequests(const BusyInfo& info) {
    CAVE_PROFILE_EVENT();

    const int budget = ComputeBudget(info);
    if (budget <= 0 || m_pending.empty()) {
        return;
    }

    int submitted = 0;
    while (!m_pending.empty() && submitted < budget) {
        const PendingRequest pending = m_pending.front();
        m_pending.pop_front();

        auto it = m_thumbnail_cache.find(pending.key);
        if (it == m_thumbnail_cache.end()) continue;

        ThumbnailRecord& rec = it->second;

        if (rec.generation != pending.generation) {
            continue;
        }

        if (rec.state == ThumbnailState::Ready || rec.state == ThumbnailState::Pending) {
            continue;
        }

        m_view_manager.submit(rec.view_desc);

        // save record
        rec.state = ThumbnailState::Pending;
        rec.submitted_frame = m_frame_index;

        m_inflight.push_back(pending.key);
        ++submitted;

#if USING(USE_LOG)
        auto handle = AssetRegistry::singleton().findByGuid(pending.key.guid);
        const AssetMetaData* meta = handle.unwrap().meta();
        LOG_TRACE(LogChannel::Thumb, "Submit '{}'", meta->name);
#endif
    }
}

void ThumbnailService::invalidate(const Guid& guid) {
    unused(guid);
}

}  // namespace cave
