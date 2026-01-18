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

ViewerTab::ViewerTab(EditorLayer& p_editor, Viewer& p_viewer)
    : m_id(TabId::Next())
    , m_editor(p_editor)
    , m_viewer(p_viewer) {
}

void ViewerTab::SelectEntity(ecs::Entity p_selected) {
    m_selected = p_selected;
    if (Scene* scene = GetScene(); scene) {
        scene->m_selected = m_selected;
    }
}

void ViewerTab::OnCreate(const Guid& p_guid) {
    auto handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();
    auto meta = handle.GetMeta();
    DEV_ASSERT(meta);

    m_title = std::format("{}###{}", meta->name, handle.GetGuid().ToString());

    LOG_OK("ViewerTab '{}' created", m_title);
}

void ViewerTab::CreateDefaultCamera2D(CameraComponent& p_out) {
    const auto res = DVAR_GET_IVEC2(resolution);
    p_out.SetOrthoFlag();
    p_out.SetView2dFlag();
    p_out.SetDimension(res.x, res.y);
    p_out.SetNear(1.0f);
    p_out.SetFar(1000.0f);
    p_out.SetPosition(Vector3f(0, 0, 10));
    p_out.SetDirtyFlag();
    p_out.Update();
}

void ViewerTab::CreateDefaultCamera3D(CameraComponent& p_out) {
    const auto res = DVAR_GET_IVEC2(resolution);
    auto camera = std::make_shared<CameraComponent>();
    p_out.SetDimension(res.x, res.y);
    p_out.SetNear(1.0f);
    p_out.SetFar(1000.0f);
    p_out.SetPosition(Vector3f(0, 4, 10));
    p_out.SetDirtyFlag();
    p_out.Update();
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

void ViewerTab::CameraInputState2D(float p_timestep,
                                   const ViewportInput& p_input,
                                   CameraInputState& p_out_state) {
    unused(p_timestep);
    unused(p_input);
    unused(p_out_state);
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
    if (!p_focused) {
    }

    CameraInputState state;
    CameraInputState3D(p_timestep, p_input, state);

#if 0
    const float timestep = m_editor.context.timestep;
    //auto& camera = active_tab->GetActiveCamera();
    CameraComponent camera;
    const auto& c = m_camera_input;
    const bool is_2d = camera.HasView2dFlag();
    if (is_2d) {
        const float speed = timestep * 0.5f;
        const float dx = speed * -c.mouse_move.x;
        const float dy = speed * c.mouse_move.y;
        CameraInputState state = {
            .move = Vector3f(dx, dy, 0.0f),
            .zoomDelta = -timestep * c.scroll,
            .rotation = Vector2f::Zero,
        };
        m_controller_2d.Update(camera, state);
        camera.Update();
    }
#endif

    // @TODO: change camera based on game mode, etc

    CameraControllerFPS m_controller_3d;
    CameraComponent& camera = const_cast<CameraComponent&>(GetActiveCameraInternal());
    m_controller_3d.Update(camera, state);
    camera.Update();
}

void ViewerTab::BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) {
    // @TODO: refactor this part
    const CameraComponent& camera = GetActiveCameraInternal();

    SceneView scene_view;
    scene_view.scene = GetScene();
    ViewInfo::FromCamera(camera, scene_view.view_info, p_is_opengl);

    p_out_views.push_back(scene_view);
}

}  // namespace cave
