// =============================================================================
// File: public/cave/render/ViewDesc.h
// =============================================================================
#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/core/math/Rect.h"
#include "cave/runtime/scene/CameraComponent.h"

namespace cave::render {

using RenderTargetId = uint32_t;

struct ViewOutputDesc {
    RenderTargetId color{};
    RenderTargetId depth{};
    bool clear_color = true;
    bool clear_depth = true;
};

struct CameraSource {
    enum class Source : uint8_t {
        Editor,
        MainCamera,
    } source;
    CameraComponent camera;

    static CameraSource Editor(const CameraComponent& p_camera) {
        return { Source::Editor, p_camera };
    }

    static CameraSource MainCamera() {
        return { Source::MainCamera };
    }
};

struct ViewHighlight {
    std::unordered_set<ecs::Entity> entities;
};

struct ViewDesc {
    CameraSource camera_source;
    SceneId scene_id;
    math::IntRect viewport_pixel;
    ViewOutputDesc output;
    // missing: which render target info it draws to

    ViewHighlight highlight;
};

}  // namespace cave::render