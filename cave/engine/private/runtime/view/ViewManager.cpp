#include "ViewManager.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
// @TODO: remove
#include "engine/private/renderer/graphics_defines.h"

namespace cave {

ViewManager::ViewManager(SceneRegistry& scene_reg, bool is_opengl) noexcept
    : scene_reg_(scene_reg)
    , is_opengl_(is_opengl) {}

void ViewManager::beginFrame() {
    DEV_ASSERT(can_submit_ == false);
    DEV_ASSERT(view_descs_.empty());
    can_submit_ = true;
}

static ResolvedView ResolveView(ViewDesc&& view_desc,
                                Scene* scene,
                                bool is_opengl) {
    using math::Mat4f;

    // https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html
    auto reverse_z = [](math::Mat4f& perspective) {
        constexpr math::Mat4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                      0.0f, 1.0f, 0.0f, 0.0f,
                                      0.0f, 0.0f, -1.0f, 0.0f,
                                      0.0f, 0.0f, 1.0f, 1.0f };
        perspective = matrix * perspective;
    };
    auto normalize_unit_range = [](math::Mat4f& perspective) {
        constexpr math::Mat4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                      0.0f, 1.0f, 0.0f, 0.0f,
                                      0.0f, 0.0f, 0.5f, 0.0f,
                                      0.0f, 0.0f, 0.5f, 1.0f };
        perspective = matrix * perspective;
    };

    const CameraComponent* cam = nullptr;
    switch (view_desc.camera_source.source) {
        case CameraSource::Source::External: {
            cam = &view_desc.camera_source.camera;
        } break;
        case CameraSource::Source::FirstCamera: {
            for (auto [id, camera] : scene->view<CameraComponent>()) {
                // @HACK: just use the first camera
                if (id.IsValid()) {
                    cam = &camera;
                    break;
                }
            }
        } break;
    }
    Mat4f view;
    Mat4f proj;
    float fovy_rad = 0.0f;
    if (cam) {
        view = cam->viewMatrix();
        proj = cam->projectionMatrix();
        fovy_rad = glm::radians(cam->fovy());
    }

    math::Frustum frustum(proj * view);

    if (is_opengl) {
        normalize_unit_range(proj);
    }
    reverse_z(proj);

    return {
        .view_id = view_desc.view_id,
        .cam = {
            .view = view,
            .proj = proj,
            .view_inv = glm::inverse(view),
            .proj_inv = glm::inverse(proj),
        },
        .frustum = frustum,
        .viewport_px = view_desc.viewport_px,
        .fovy_rad = fovy_rad,
        .scene_id = view_desc.scene_id,
        .scene = scene,
        .highlight = std::move(view_desc.highlight),
        .output = std::move(view_desc.output),
    };
}

std::span<const ResolvedView> ViewManager::endFrame() {
    DEV_ASSERT(can_submit_ == true);
    can_submit_ = false;

    resolved_views_.clear();
    resolved_views_.reserve(view_descs_.size());
    for (ViewDesc& desc : view_descs_) {
        SceneId id = desc.scene_id;
        if (Scene* scene = scene_reg_.resolve(id)) {
            resolved_views_.emplace_back(ResolveView(std::move(desc), scene, is_opengl_));
        } else {
            LOG_ERROR(LogChannel::View, "can't resolve {}#{}", id.index, id.gen);
        }
    }

    view_descs_.clear();
    return resolved_views_;
}

void ViewManager::submit(const ViewDesc& view_desc) {
    DEV_ASSERT(can_submit_);
    view_descs_.emplace_back(view_desc);
}

ViewId ViewManager::createView(std::string_view debug_name,
                               const math::IntRect& viewport_px) {
    auto view = std::make_unique<ViewRecord>();
    view->debug_name = debug_name;
    view->viewport_fb = viewport_px;
    const ViewId id = Base::Create(std::move(view));
    LOG_TRACE(LogChannel::View, "+{} id=({},{})", debug_name, id.index, id.gen);
    return id;
}

void ViewManager::destroyView(ViewId view_id) {
    Base::Destroy(view_id);
    LOG_TRACE(LogChannel::View, "-View id=({},{})", view_id.index, view_id.gen);
}

}  // namespace cave
