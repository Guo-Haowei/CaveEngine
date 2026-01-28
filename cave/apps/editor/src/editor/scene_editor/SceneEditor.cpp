#include "SceneEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/runtime/framework/IApplication.h"

#include "editor/edit/EditTransformCmd.h"
#include "editor/services/EditService.h"
#include "editor/services/SelectionService.h"

// @TODO: refactor
#include "editor/document/DocumentService.h"
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/runtime/scene/EntityFactory.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"

// #include "editor/document/document.h"
#include "editor/document/SceneDocument.h"
#include "editor/EditorState.h"
#include "editor/utility/ImGuizmo.h"
#include "editor/viewer/Viewer.h"

#include "editor/EditorDvars.h"

namespace cave {

SceneEditor::SceneEditor(EditorState& p_editor,
                         DocId p_doc_id,
                         SceneId p_preview_scene_id,
                         ViewDimension p_dimension)
    : Tab(p_editor, p_doc_id, p_dimension)
    , m_preview_scene(p_preview_scene_id)
    , m_button_displays{ ICON_FA_PLAY, ICON_FA_PAUSE }
    , m_button_tooltips{ "Run Project", "Pause Project" } {

    m_play_button = {
        ICON_FA_PLAY,
        "Run Project",
        [this]() {
            m_editor.RequestModeSwitch();
            m_button_index = 1 - m_button_index;
            m_play_button.display = m_button_displays[m_button_index];
            m_play_button.tooltip = m_button_tooltips[m_button_index];
        }
    };
}

void SceneEditor::BuildViewsImpl(SceneId p_scene_id,
                                 ecs::Entity p_camera,
                                 std::vector<SceneView>& p_out_views,
                                 bool p_is_opengl) {
    // @TODO: refactor scene view API
    if (m_editor.IsPlaying()) {
        DEV_ASSERT(0);
        SceneView scene_view;
        scene_view.scene_id = m_editor.GetRuntimeHost().GetSceneId();
        scene_view.scene_manager = m_editor.GetApp().GetSceneRegistry();

        Scene* scene = scene_view.ResolveScene();

        // @HACK: find the first non-editor camera
        for (auto [id, camera] : scene->View<CameraComponent>()) {
            if (scene->Contains<NoSaveTag>(id)) {
                continue;
            }

            ViewInfo::FromCamera(camera,
                                 scene_view.view_info,
                                 p_is_opengl);

            p_out_views.push_back(scene_view);
            break;
        }
        return;
    }

    // @HACK: force update
    SceneView scene_view;
    scene_view.scene_id = p_scene_id;
    scene_view.scene_manager = m_editor.GetApp().GetSceneRegistry();

    Scene* scene = scene_view.ResolveScene();
    const CameraComponent* cam = scene->GetComponent<CameraComponent>(p_camera);

    if (DEV_VERIFY(cam)) {
        ViewInfo::FromCamera(*cam,
                             scene_view.view_info,
                             p_is_opengl);

        p_out_views.push_back(scene_view);
    }
}

void SceneEditor::OnCreate() {
    switch (m_dim) {
        case DIMENSION_2: {
            SetupDefault2DCamera();
        } break;
        case DIMENSION_3: {
            SetupDefault3DCamera();
        } break;
    }

    IApplication& app = m_editor.GetApp();
    app.GetSceneScheduler().Register(this);
}

void SceneEditor::OnDestroy() {
    IApplication& app = m_editor.GetApp();
    app.GetSceneScheduler().Unregister(this);
}

void SceneEditor::CollectSceneTicks(std::vector<SceneTickRequest>& p_out) {
    if (!m_editor.IsPlaying()) {
        p_out.push_back(SceneTickRequest{
            SceneTickMode::Editor,
            m_preview_scene,
        });
    }
}

// @TODO: rename this to DrawEditor
#if 0
void SceneEditor::DrawMainView(const CameraComponent&) {
    CameraComponent& camera = *scene.GetComponent<CameraComponent>(m_camera);

    DocId doc_id = GetDocId();

    ViewerTab::DrawMainView(camera);

    const Matrix4x4f& view_matrix = camera.GetViewMatrix();
    const Matrix4x4f& proj_matrix = camera.GetProjectionMatrix();
    const Matrix4x4f& proj_view = camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    SelectionKey selection = m_editor.SelectionService().Primary(m_doc_id);
    ecs::Entity id = selection.entity;
    TransformComponent* transform_component = scene.GetComponent<TransformComponent>(id);

    EditService& edit_service = m_editor.EditService();

    auto draw_gizmo = [&](ImGuizmo::OPERATION p_operation) {
        if (transform_component) {
            const Matrix4x4f before = transform_component->GetLocalMatrix();
            Matrix4x4f after = before;
            if (ImGuizmo::Manipulate(glm::value_ptr(view_matrix),
                                     glm::value_ptr(proj_matrix),
                                     p_operation,
                                     ImGuizmo::LOCAL,
                                     // ImGuizmo::WORLD,
                                     glm::value_ptr(after),
                                     nullptr, nullptr, nullptr, nullptr)) {

                auto cmd = std::make_unique<EditTransformCmd>(m_editor.GetApp(),
                                                              id,
                                                              before,
                                                              after);
                edit_service.Submit(doc_id, std::move(cmd));
            }
        }
    };

    switch (m_state) {
        case GizmoAction::Translate:
            draw_gizmo(ImGuizmo::TRANSLATE);
            break;
        case GizmoAction::Rotate:
            draw_gizmo(ImGuizmo::ROTATE);
            break;
        case GizmoAction::Scale:
            draw_gizmo(ImGuizmo::SCALE);
            break;
        default:
            break;
    }

    // @TODO: make show_editor as viewer attribute
    // drag grid, grid size, snap, etc
    const bool show_editor = DVAR_GET_BOOL(show_editor);
    if (show_editor) {
        ImGuizmo::DrawAxes(proj_view);

        DEV_ASSERT(0);
        //const float size = 120.f;
        //const auto& min = m_viewer.GetCanvasMin();
        //ImGuizmo::ViewManipulate((float*)&view_matrix[0].x,
        //                         10.0f,
        //                         ImVec2(min.x, min.y),
        //                         ImVec2(size, size),
        //                         IM_COL32(64, 64, 64, 96));
    }
}
#endif

#if 0
bool SceneEditor::HandleInput(const OldInputEvent* p_input_event) {
    // select
    if (auto e = dynamic_cast<const InputEventMouse*>(p_input_event); e) {
        if (e->IsButtonPressed(MouseButton::RIGHT)) {
            Vector2f clicked = e->GetPos();
            Select(clicked);
            return true;
        }
    }
    DEV_ASSERT(0);

    return false;
}
#endif

// const std::vector<const ToolBarButtonDesc*> SceneEditor::GetToolBarButtons() const {
//     return { &m_play_button };
// }

#if 0
void SceneEditor::Select(const Vector2f& p_cursor) {
    unused(p_cursor);
    DEV_ASSERT(0);
    if (auto res = m_viewer.CursorToNDC(p_cursor); res.is_some()) {
        Vector2f ndc_2 = res.unwrap_unchecked();
        Vector4f ndc{ ndc_2.x, ndc_2.y, 1.0f, 1.0f };

        CameraComponent cam;

        const Matrix4x4f inv_pv = glm::inverse(cam.GetProjectionViewMatrix());

        const Vector3f ray_start = Vector3f::Zero;
        // const Vector3f ray_start = m_camera_transform.GetTranslation();
        const Vector3f direction = normalize(Vector3f((inv_pv * ndc).xyz));
        const Vector3f ray_end = ray_start + direction * cam.GetFar();
        Ray ray(ray_start, ray_end);

        const auto result = GetScene()->Intersects(ray);
        SetSelectedEntity(result.entity);
    }
}
#endif

Scene* SceneEditor::GetResolvedScene() {
    return m_editor.GetApp().GetSceneRegistry()->Resolve(m_preview_scene);
}

static const char EDITOR_CAMERA_NAME[] = "_editor_cam";

void SceneEditor::SetupDefault2DCamera() {
    Scene* scene = GetResolvedScene();
    DEV_ASSERT(scene);

    ecs::Entity cam = scene->FindEntityByName(EDITOR_CAMERA_NAME);
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

void SceneEditor::SetupDefault3DCamera() {
    Scene* scene = GetResolvedScene();
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

CameraInputState SceneEditor::CreateCameraInputState2D(const std::vector<InputEvent>& p_events, const KeyState&) {
    CameraInputState state{};

    float dx = 0.0f;
    float dy = 0.0f;
    bool mmb = false;

    for (const InputEvent& e : p_events) {
        if (e.consumed) {
            continue;
        }

        switch (e.type) {
            case InputEventType::MouseWheel: {
                e.consumed = true;
                state.zoom_delta = -e.dy;
            } break;
            case InputEventType::MouseMove: {
                e.consumed = true;
                dx = -e.dx;
                dy = e.dy;
            } break;
            case InputEventType::ButtonDown:
                if (e.code == std::to_underlying(Key::MMB)) {
                    e.consumed = true;
                    mmb = true;
                }
                break;
            default:
                break;
        }

        if (mmb) {
            state.move = Vector3f(dx, dy, 0.0f);
        }
    }

    return state;
}

CameraInputState SceneEditor::CreateCameraInputState3D(const std::vector<InputEvent>& p_events, const KeyState& p_st) {
    Vector2f rotation = Vector2f::Zero;

    const InputDeviceId id{ 0 };
    const bool mmb = p_st.Down(id, Key::MMB);
    const int dx = p_st.Down(id, Key::D) - p_st.Down(id, Key::A);
    const int dy = p_st.Down(id, Key::E) - p_st.Down(id, Key::Q);
    const int dz = p_st.Down(id, Key::W) - p_st.Down(id, Key::S);

    CameraInputState state{};

    for (const InputEvent& e : p_events) {
        if (e.consumed) {
            continue;
        }
        switch (e.type) {
            case InputEventType::MouseWheel: {
                e.consumed = true;
                state.zoom_delta = 3.0f * e.dy;
            } break;
            case InputEventType::MouseMove: {
                if (mmb) {
                    e.consumed = true;
                    state.rotation.x = e.dx;
                    state.rotation.y = e.dy;
                }
            } break;
            default:
                break;
        }
    }

    state.move = Vector3f(dx, dy, dz);
    return state;
}

}  // namespace cave
