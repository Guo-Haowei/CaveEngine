#include "viewer_tab.h"

#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#include "editor/document/document.h"
#include "editor/editor_state.h"
#include "editor/panels/asset_inspector.h"
#include "editor/viewer/viewer.h"

namespace cave {

using ecs::Entity;

static const char EDITOR_CAMERA_NAME[] = "_editor_cam";

const Guid& ViewerTab::GetGuid() const {
    return GetDocument().GetGuid();
}

ViewerTab::ViewerTab(EditorState& p_editor, Viewer& p_viewer, Dimension p_dimension)
    : m_id(TabId::Next())
    , m_dimension(p_dimension)
    , m_editor(p_editor)
    , m_viewer(p_viewer) {
}

void ViewerTab::SetSelectedEntity(ecs::Entity p_selected) {
    m_selected = p_selected;
    if (Scene* scene = GetScene(); scene) {
        scene->m_selected = m_selected;
    }
}

void ViewerTab::SetCopiedEntity(ecs::Entity p_copied) {
    m_copied = p_copied;
}

void ViewerTab::OnCreate(const Guid& p_guid) {
    auto handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();
    auto meta = handle.GetMeta();
    DEV_ASSERT(meta);

    m_title = std::format("{}###{}", meta->name, handle.GetGuid().ToString());

    LOG_OK("ViewerTab '{}' created", m_title);

    OnCreateInternal(p_guid);

    switch (m_dimension) {
        case DIMENSION_2: {
            SetupDefault2DCamera();
        } break;
        case DIMENSION_3: {
            SetupDefault3DCamera();
        } break;
    }
}

void ViewerTab::SetupDefault2DCamera() {
    Scene* scene = GetScene();
    DEV_ASSERT(scene);

    Entity cam = scene->FindEntityByName(EDITOR_CAMERA_NAME);
    if (!cam.IsValid()) {
        cam = EntityFactory::CreateCameraEntity(*scene, EDITOR_CAMERA_NAME);
        scene->Create<NoSaveTag>(cam);
        scene->AttachChild(cam);
        CameraComponent* camera = scene->GetComponent<CameraComponent>(cam);
        camera->SetProjection(ProjectionType::Orthographic);
        TransformComponent* transform = scene->GetComponent<TransformComponent>(cam);
        transform->SetTranslation(Vector3f(0, 0, 10));
    }

    m_camera = cam;
    m_camera_controller = std::make_shared<CameraController2DEditor>(scene, cam);
}

void ViewerTab::SetupDefault3DCamera() {
    Scene* scene = GetScene();
    DEV_ASSERT(scene);

    Entity cam = scene->FindEntityByName(EDITOR_CAMERA_NAME);
    Entity cam_y = scene->FindEntityByName("_editor_cam_y");
    Entity cam_root = scene->FindEntityByName("_editor_cam_root");

    if (!cam.IsValid()) {
        cam = EntityFactory::CreateCameraEntity(*scene, EDITOR_CAMERA_NAME);
        cam_y = EntityFactory::CreateTransformEntity(*scene, "_editor_cam_y");
        cam_root = EntityFactory::CreateTransformEntity(*scene, "_editor_cam_root");

        scene->Create<NoSaveTag>(cam);
        scene->Create<NoSaveTag>(cam_y);
        scene->Create<NoSaveTag>(cam_root);

        scene->AttachChild(cam_root);
        scene->AttachChild(cam_y, cam_root);
        scene->AttachChild(cam, cam_y);
    }

    m_camera = cam;
    m_camera_controller = std::make_shared<CameraControllerFPS>(scene, cam_root, cam_y, cam);
}

void ViewerTab::OnActivate() {
    m_active = true;
    OnActivateInternal();
}

void ViewerTab::OnDeactivate() {
    OnDeactivateInternal();
    m_active = false;
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
    const auto& gm = *m_editor.GetApp().GetGraphicsManager();
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
    const int dx = p_input.IsKeyDown(Key::KEY_D) - p_input.IsKeyDown(Key::KEY_A);
    const int dy = p_input.IsKeyDown(Key::KEY_E) - p_input.IsKeyDown(Key::KEY_Q);
    const int dz = p_input.IsKeyDown(Key::KEY_W) - p_input.IsKeyDown(Key::KEY_S);

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

    m_camera_controller->Update(state);
}

void ViewerTab::BuildViewsImpl(Scene* p_scene,
                               ecs::Entity p_camera,
                               std::vector<SceneView>& p_out_views,
                               bool p_is_opengl) {

    const CameraComponent* cam = p_scene->GetComponent<CameraComponent>(p_camera);

    if (DEV_VERIFY(cam)) {
        SceneView scene_view;
        scene_view.scene = p_scene;

        ViewInfo::FromCamera(*cam,
                             scene_view.view_info,
                             p_is_opengl);

        p_out_views.push_back(scene_view);
    }
}

void ViewerTab::BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) {
    if (!m_active) {
        return;
    }

    BuildViewsImpl(GetScene(), m_camera, p_out_views, p_is_opengl);
}

}  // namespace cave
