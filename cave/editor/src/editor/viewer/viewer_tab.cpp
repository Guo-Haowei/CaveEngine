#include "viewer_tab.h"

#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"

#include "editor/document/document.h"
#include "editor/editor_layer.h"
#include "editor/panels/asset_inspector.h"
#include "editor/viewer/viewer.h"

namespace cave {

const Guid& ViewerTab::GetGuid() const {
    return GetDocument().GetGuid();
}

ViewerTab::ViewerTab(EditorLayer& p_editor, Viewer& p_viewer, Dimension p_dimension)
    : m_id(TabId::Next())
    , m_dimension(p_dimension)
    , m_editor(p_editor)
    , m_viewer(p_viewer) {
}

void ViewerTab::SelectEntity(ecs::Entity p_selected) {
    m_selected = p_selected;
    if (Scene* scene = GetScene(); scene) {
        scene->m_selected = m_selected;
    }
}

static std::shared_ptr<CameraComponent> CreateDefaultCamera2D(const Vector2i& p_resolution, TransformComponent& p_transform) {
    auto camera = std::make_shared<CameraComponent>();
    camera->SetProjection(ProjectionType::Orthographic);
    camera->SetDimension(p_resolution.x, p_resolution.y);
    camera->SetNear(1.0f);
    camera->SetFar(1000.0f);
    p_transform.SetTranslation(Vector3f(0, 0, 10));

    camera->Update(p_transform.GetWorldMatrix());
    return camera;
}

static std::shared_ptr<CameraComponent> CreateDefaultCamera3D(const Vector2i& p_resolution, TransformComponent& p_transform) {
    auto camera = std::make_shared<CameraComponent>();
    camera->SetDimension(p_resolution.x, p_resolution.y);
    camera->SetNear(1.0f);
    camera->SetFar(1000.0f);

    p_transform.SetTranslation(Vector3f(0, 4, 10));
    camera->Update(p_transform.GetWorldMatrix());
    return camera;
}

void ViewerTab::OnCreate(const Guid& p_guid) {
    auto handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();
    auto meta = handle.GetMeta();
    DEV_ASSERT(meta);

    m_title = std::format("{}###{}", meta->name, handle.GetGuid().ToString());

    LOG_OK("ViewerTab '{}' created", m_title);

    // create camera & controller
    const bool is_2d = m_dimension == DIMENSION_2;

    const Vector2i res = DVAR_GET_IVEC2(resolution);
    m_camera = is_2d ? CreateDefaultCamera2D(res, m_camera_transform) : CreateDefaultCamera3D(res, m_camera_transform);

    if (is_2d) {
        m_camera_controller = std::make_shared<CameraController2DEditor>();
    } else {
        m_camera_controller = std::make_shared<CameraControllerFPS>();
    }

    // CreateDefaultCameraAndController();

    OnCreateInternal(p_guid);
}

void ViewerTab::OnActivate() {
    m_active = true;
    OnActivateInternal();
}

void ViewerTab::OnDeactivate() {
    OnDeactivateInternal();
    m_active = false;
}

void ViewerTab::CreateDefaultCameraAndController() {
    // @TODO: controller
}

void ViewerTab::DrawAssetInspector() {
    m_editor.GetAssetInspector().DrawContentBrowser();
}

void ViewerTab::DrawMainView(const CameraComponent&) {
    const auto canvas_min = m_viewer.GetCanvasMin();
    const auto canvas_max = canvas_min + m_viewer.GetCanvasSize();

    ImVec2 top_left(canvas_min.x, canvas_min.y);
    ImVec2 bottom_right(canvas_max.x, canvas_max.y);

    // @TODO: fix this
    const auto& gm = *m_editor.GetApplication()->GetGraphicsManager();
    uint64_t handle = gm.GetFinalImage();
    // add image for drawing
    switch (gm.GetBackend()) {
        case Backend::D3D11:
        case Backend::D3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right);
        } break;
        case Backend::OPENGL: {
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            if (gm.GetActiveRenderGraphName() == RenderGraphName::PATHTRACER) {
                uv_min = ImVec2(0, 0);
                uv_max = ImVec2(1, 1);
            }
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right, uv_min, uv_max);
        } break;
        case Backend::VULKAN:
        case Backend::METAL: {
        } break;
        default:
            CRASH_NOW();
            break;
    }
}

// @TODO: refactor
void ViewerTab::CameraInputState2D(float p_timestep,
                                   const ViewportInput& p_input,
                                   CameraInputState& p_out_state) {
    const float speed = p_timestep * 0.5f;
    const float dx = speed * -p_input.mouse_move.x;
    const float dy = speed * p_input.mouse_move.y;
    p_out_state.zoom_delta = -p_timestep * p_input.wheel_delta;
    p_out_state.rotation = Vector2f::Zero;
    if (p_input.IsButtonDown(MouseButton::MIDDLE)) {
        p_out_state.move = Vector3f(dx, dy, 0.0f);
    }
}

void ViewerTab::CameraInputState3D(float p_timestep,
                                   const ViewportInput& p_input,
                                   CameraInputState& p_out_state) {
    const int dx = p_input.IsKeyDown(KeyCode::KEY_D) - p_input.IsKeyDown(KeyCode::KEY_A);
    const int dy = p_input.IsKeyDown(KeyCode::KEY_E) - p_input.IsKeyDown(KeyCode::KEY_Q);
    const int dz = p_input.IsKeyDown(KeyCode::KEY_W) - p_input.IsKeyDown(KeyCode::KEY_S);

    p_out_state.move = p_timestep * Vector3f(dx, dy, dz);
    p_out_state.zoom_delta = p_timestep * 3.0f * p_input.wheel_delta;

    if (p_input.IsButtonDown(MouseButton::MIDDLE)) {
        p_out_state.rotation = p_timestep * p_input.mouse_move;
    }
}

void ViewerTab::Update(float p_timestep,
                       const ViewportInput& p_input,
                       bool p_focused) {
    if (!m_viewer.IsHovered()) {
        return;
    }

    if (!p_focused) {
    }

    CameraInputState state;

    switch (m_dimension) {
        case DIMENSION_2: {
            CameraInputState2D(p_timestep, p_input, state);
        } break;
        case DIMENSION_3: {
            CameraInputState3D(p_timestep, p_input, state);
        } break;
        default: {
            CRASH_NOW_MSG("invalid dimension");
        } break;
    }

    m_camera_controller->Update(state, *m_camera, m_camera_transform);

    m_camera_transform.SetDirty();
    m_camera_transform.UpdateTransform();

    m_camera->Update(m_camera_transform.GetWorldMatrix());
}

void ViewerTab::BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) {
    if (!m_active) {
        return;
    }

    SceneView scene_view;
    scene_view.scene = GetScene();

    ViewInfo::FromCamera(*m_camera,
                         m_camera_transform.GetWorldMatrix(),
                         scene_view.view_info,
                         p_is_opengl);

    p_out_views.push_back(scene_view);
}

}  // namespace cave
