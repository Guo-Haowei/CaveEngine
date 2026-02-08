// =============================================================================
// File: public/cave/render/ViewDesc.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/math/Rect.h"
#include "cave/runtime/scene/CameraComponent.h"

namespace cave {
struct GpuTexture;
using GpuTextureId = std::shared_ptr<GpuTexture>;
}  // namespace cave

namespace cave::render {

using RenderTargetId = uint32_t;

struct ViewOutputDesc {
    RenderTargetId color{};
    RenderTargetId depth{};
    bool clear_color = true;
    bool clear_depth = true;
};

// @TODO: do not pass CameraComponent here
struct CameraSource {
    enum class Source : uint8_t {
        External,
        FirstCamera,
    } source;
    CameraComponent camera;

    static CameraSource External(const CameraComponent& p_camera) {
        return { Source::External, p_camera };
    }

    static CameraSource FirstCamera() {
        return { Source::FirstCamera };
    }
};

struct ViewHighlight {
    std::unordered_set<ecs::Entity> entities;
};

struct ViewDesc {
    CameraSource camera_source;
    SceneId scene_id;
    math::IntRect viewport_px;

    ViewHighlight highlight;
    GpuTextureId output;
};

}  // namespace cave::render