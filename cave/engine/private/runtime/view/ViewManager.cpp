#include "ViewManager.h"

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
// @TODO: remove
#include "engine/private/renderer/graphics_defines.h"

namespace cave {

ViewManager::ViewManager(SceneRegistry& scene_reg, bool is_opengl) noexcept
    : m_scene_reg(scene_reg)
    , m_is_opengl(is_opengl) {}

void ViewManager::beginFrame() {
    DEV_ASSERT(m_can_submit == false);
    DEV_ASSERT(m_view_descs.empty());
    m_can_submit = true;
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
                if (id.valid()) {
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
    DEV_ASSERT(m_can_submit == true);
    m_can_submit = false;

    m_resolved_views.clear();
    m_resolved_views.reserve(m_view_descs.size());
    for (ViewDesc& desc : m_view_descs) {
        SceneId id = desc.scene_id;
        if (Scene* scene = m_scene_reg.resolve(id)) {
            m_resolved_views.emplace_back(ResolveView(std::move(desc), scene, m_is_opengl));
        }
#if 0
        else {
            LOG_ERROR(LogChannel::View, "can't resolve {}#{}", id.index, id.gen);
        }
#endif
    }

    m_view_descs.clear();
    return m_resolved_views;
}

void ViewManager::submit(const ViewDesc& view_desc) {
    DEV_ASSERT(m_can_submit);
    m_view_descs.emplace_back(view_desc);
}

ViewId ViewManager::createView(std::string_view debug_name,
                               const math::IntRect& viewport_px) {
    auto view = MakeOwner<ViewRecord>();
    view->debug_name = debug_name;
    view->viewport_fb = viewport_px;
    const ViewId id = Base::create(std::move(view));
    LOG_TRACE(LogChannel::View, "+{} {}", debug_name, id.toString());
    return id;
}

void ViewManager::destroyView(ViewId view_id) {
    Base::destroy(view_id);
    LOG_TRACE(LogChannel::View, "-View {}", view_id.toString());
}

}  // namespace cave
