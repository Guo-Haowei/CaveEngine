#include "ViewManager.h"

#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/SceneRegistry.h"
// @TODO: remove
#include "engine/private/renderer/graphics_defines.h"

namespace cave {

ViewManager::ViewManager()
    : IService("ViewManager") {}

auto ViewManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void ViewManager::FinalizeImpl() {
}

void ViewManager::BeginFrame() {
    DEV_ASSERT(m_can_submit == false);
    DEV_ASSERT(m_view_descs.empty());
    m_can_submit = true;
}

static ResolvedView ResolveView(ViewDesc&& p_view,
                                Scene* p_scene,
                                bool p_is_opengl) {
    using math::Matrix4x4f;

    // https://tomhultonharrop.com/mathematics/graphics/2023/08/06/reverse-z.html
    auto reverse_z = [](math::Matrix4x4f& p_perspective) {
        constexpr math::Matrix4x4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, -1.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f, 1.0f };
        p_perspective = matrix * p_perspective;
    };
    auto normalize_unit_range = [](math::Matrix4x4f& p_perspective) {
        constexpr math::Matrix4x4f matrix{ 1.0f, 0.0f, 0.0f, 0.0f,
                                           0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 0.5f, 0.0f,
                                           0.0f, 0.0f, 0.5f, 1.0f };
        p_perspective = matrix * p_perspective;
    };

    const CameraComponent* cam = nullptr;
    switch (p_view.camera_source.source) {
        case CameraSource::Source::External: {
            cam = &p_view.camera_source.camera;
        } break;
        case CameraSource::Source::FirstCamera: {
            for (auto [id, camera] : p_scene->View<CameraComponent>()) {
                // @HACK: just use the first camera
                if (id.IsValid()) {
                    cam = &camera;
                    break;
                }
            }
        } break;
    }
    Matrix4x4f view;
    Matrix4x4f proj;
    float fovy_rad = 0.0f;
    if (cam) {
        view = cam->GetViewMatrix();
        proj = cam->GetProjectionMatrix();
        fovy_rad = glm::radians(cam->GetFovy());
    }

    math::Frustum frustum(proj * view);

    if (p_is_opengl) {
        normalize_unit_range(proj);
    }
    reverse_z(proj);

    return {
        .view_id = p_view.view_id,
        .cam = {
            .view = view,
            .proj = proj,
            .view_inv = glm::inverse(view),
            .proj_inv = glm::inverse(proj),
        },
        .frustum = frustum,
        .viewport_px = p_view.viewport_px,
        .fovy_rad = fovy_rad,
        .scene_id = p_view.scene_id,
        .scene = p_scene,
        .highlight = std::move(p_view.highlight),
        .output = std::move(p_view.output),
    };
}

std::span<const ResolvedView> ViewManager::EndFrame() {
    DEV_ASSERT(m_can_submit == true);
    m_can_submit = false;

    const bool is_opengl = m_app->IsOpenGL();

    m_views.clear();
    m_views.reserve(m_view_descs.size());
    for (ViewDesc& desc : m_view_descs) {
        SceneId id = desc.scene_id;
        if (Scene* scene = m_app->GetSceneRegistry()->Resolve(id)) {
            m_views.emplace_back(ResolveView(std::move(desc), scene, is_opengl));
        } else {
            LOG_ERROR("can't resolve scene ({},{})", id.index, id.gen);
        }
    }

    m_view_descs.clear();
    return m_views;
}

void ViewManager::Submit(const ViewDesc& p_view_desc) {
    DEV_ASSERT(m_can_submit);
    m_view_descs.emplace_back(p_view_desc);
}

ViewId ViewManager::Create(std::string_view p_debug_name) {
    auto view = std::make_unique<ViewRecord>();
    view->debug_name = p_debug_name;
    const ViewId id = Base::Create(std::move(view));
    LOG_VERBOSE("ViewManager: View '{}'({},{}) created.", p_debug_name, id.index, id.gen);
    return id;
}

void ViewManager::Destroy(ViewId p_view_id) {
    Base::Destroy(p_view_id);
    LOG_VERBOSE("ViewManager: ViewId({},{}) destroyed.", p_view_id.index, p_view_id.gen);
}

}  // namespace cave
