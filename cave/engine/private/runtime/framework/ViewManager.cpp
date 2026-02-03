#include "ViewManager.h"

#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
// @TODO: remove
#include "engine/private/renderer/graphics_defines.h"

namespace cave {

using render::CameraSource;
using render::ResolvedView;
using render::ViewDesc;

ViewManager::ViewManager()
    : Module("ViewManager") {}

auto ViewManager::InitializeImpl() -> Result<void> {
    return Result<void>();
}

void ViewManager::FinalizeImpl() {
}

void ViewManager::BeginFrame() {
    m_can_submit = true;
    m_view_descs.clear();
}

static ResolvedView ResolveView(const ViewDesc& p_view,
                                const Scene* p_scene,
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
        case CameraSource::Source::Editor: {
            cam = &p_view.camera_source.camera;
        } break;
        case CameraSource::Source::MainCamera: {
            for (auto [id, camera] : p_scene->View<CameraComponent>()) {
                // @HACK: just use the first camera
                if (id.IsValid()) {
                    cam = &camera;
                    break;
                }
            }
        } break;
    }
    DEV_ASSERT(cam);

    Matrix4x4f view = cam->GetViewMatrix();
    Matrix4x4f proj = cam->GetProjectionMatrix();
    math::Frustum frustum(proj * view);

    if (p_is_opengl) {
        normalize_unit_range(proj);
    }
    reverse_z(proj);

    return {
        .view = view,
        .proj = proj,
        .view_inv = glm::inverse(view),
        .proj_inv = glm::inverse(proj),
        .frustum = frustum,
        .position = cam->GetPosition(),
        .up = cam->GetUp(),
        .front = cam->GetFront(),
        .right = cam->GetRight(),
        .vp_w = static_cast<float>(cam->GetWidth()),
        .vp_h = static_cast<float>(cam->GetHeight()),
        .aspect = cam->GetAspect(),
        .fovy = cam->GetFovy(),
        .highlight = p_view.highlight,
    };
}

std::span<const render::ResolvedView> ViewManager::EndFrame() {
    m_can_submit = false;
    // @TODO: resolve view
    const bool is_opengl = m_app->GetRenderDevice()->GetBackend() == Backend::OPENGL;

    m_views.clear();
    m_views.reserve(m_view_descs.size());
    for (const ViewDesc& v : m_view_descs) {
        Scene* scene = m_app->GetSceneRegistry()->Resolve(v.scene_id);
        DEV_ASSERT(scene);
        if (!scene) continue;

        m_views.emplace_back(ResolveView(v, scene, is_opengl));
    }

    return m_views;
}

void ViewManager::Submit(const render::ViewDesc& p_view_desc) {
    DEV_ASSERT(m_can_submit);
    m_view_descs.emplace_back(p_view_desc);
}

}  // namespace cave
