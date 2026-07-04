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
    : view_manager_(services.viewManager())
    , scene_reg_(services.sceneRegistry())
    , render_device_(services.renderDevice())
    , builder_(services) {
}

uint64_t ThumbnailService::getOrRequest(const ThumbnailKey& key) {
    auto [it, inserted] = cache_.try_emplace(key);
    if (!inserted) {
        ThumbnailRecord& rec = it->second;
        rec.last_used_frame = frame_index_;
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

    PreviewBuildResult res = builder_.build(req);
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
    auto tex = render_device_.CreateTexture(
        tex_desc,
        PointClampSampler());

    ThumbnailRecord& rec = it->second;
    math::IntRect vp = { 0, 0, (int)w, (int)h };
    rec = {
        .view_desc = {
            .view_id = view_manager_.createView("ThumbnailView", vp),
            .scene_id = res.scene_id,
            .camera_source = res.camera,
            .viewport_px = vp,
            .highlight = {},
            .output = tex,
        },
        .state = ThumbnailState::Missing,
        .gpu_handle = tex->GetHandle(),
        .last_used_frame = frame_index_,
        .submitted_frame = 0,
        .generation = 1,
    };

    pending_.emplace_back(PendingRequest{ key, rec.generation });
    return 0;
}

void ThumbnailService::tick(const FrameTime& time, const BusyInfo& info) {
    processCompletions();
    frame_index_ = time.frame_index;
    submitRequests(info);
}

static int ComputeBudget(const BusyInfo& info) {
    unused(info);
    return 1;
}

void ThumbnailService::processCompletions() {
    for (const ThumbnailKey& key : inflight_) {
        auto it = cache_.find(key);
        if (it == cache_.end()) continue;

        ThumbnailRecord& rec = it->second;
        if (rec.state != ThumbnailState::Pending) continue;

        // if (rec.submitted_frame <= completed_frame_index)
        {
            rec.state = ThumbnailState::Ready;
            view_manager_.destroyView(rec.view_desc.view_id);
            scene_reg_.destroyScene(rec.view_desc.scene_id);
        }
    }

    inflight_.clear();
}

void ThumbnailService::submitRequests(const BusyInfo& info) {
    CAVE_PROFILE_EVENT();

    const int budget = ComputeBudget(info);
    if (budget <= 0 || pending_.empty()) {
        return;
    }

    int submitted = 0;
    while (!pending_.empty() && submitted < budget) {
        const PendingRequest pending = pending_.front();
        pending_.pop_front();

        auto it = cache_.find(pending.key);
        if (it == cache_.end()) continue;

        ThumbnailRecord& rec = it->second;

        if (rec.generation != pending.generation) {
            continue;
        }

        if (rec.state == ThumbnailState::Ready || rec.state == ThumbnailState::Pending) {
            continue;
        }

        view_manager_.submit(rec.view_desc);

        // save record
        rec.state = ThumbnailState::Pending;
        rec.submitted_frame = frame_index_;

        inflight_.push_back(pending.key);
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
