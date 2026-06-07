// =============================================================================
// File: cave/runtime/view/ViewDesc.h
// =============================================================================
#pragma once
#include <string_view>
#include "cave/core/ids/SceneId.h"
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Rect.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/ecs/components/CameraComponent.h"

namespace cave {

struct GpuTexture;
using GpuTextureId = std::shared_ptr<GpuTexture>;
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
    ViewId view_id{};
    SceneId scene_id{};

    CameraSource camera_source{};
    math::IntRect viewport_px{};

    ViewHighlight highlight{};
    GpuTextureId output{};
};

}  // namespace cave