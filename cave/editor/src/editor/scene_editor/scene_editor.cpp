#include "scene_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "engine/runtime/application.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#include "editor/document/document.h"
#include "editor/editor_state.h"
#include "editor/editor_scene_manager.h"
#include "editor/scene_editor/scene_document.h"
#include "editor/utility/imguizmo.h"
#include "editor/viewer/viewer.h"

// @TODO: refactor
#include "editor/editor_dvars.h"

namespace cave {

SceneEditor::SceneEditor(EditorState& p_editor, Viewer& p_viewer, ViewerTab::Dimension p_dimension)
    : ViewerTab(p_editor, p_viewer, p_dimension) {

    m_play_button = {
        ICON_FA_PLAY,
        "Run Project",
        [&]() {
            m_editor.RequestGamePlay();
        }
    };
    m_pause_button = {
        ICON_FA_PAUSE,
        "Pause Running Project",
        [&]() {
        },
    };
}

Document& SceneEditor::GetDocument() const {
    return *m_document.get();
}

void SceneEditor::OnCreateInternal(const Guid& p_guid) {
    m_document = std::make_shared<SceneDocument>(p_guid);
}

void SceneEditor::OnDestroy() {
}

void SceneEditor::OnActivateInternal() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApp().GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->OpenTempScene(m_document->m_scene);
}

Scene* SceneEditor::GetScene() {
    // if (game_mode == GameMode::Gameplay) {
    //     auto scene = app.GetSceneManager()->GetActiveScene();
    //     DEV_ASSERT(scene);
    //     return scene.get();
    // }

    auto handle = m_document->GetHandle<Scene>();
    return handle.Get();
}

void SceneEditor::BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) {
    if (!m_active) {
        return;
    }

    Scene* scene = GetScene();

    // @TODO: fix this part
    if (true) {
        // @HACK: find the first non-editor camera
        for (auto [id, camera] : scene->View<CameraComponent>()) {
            if (scene->Contains<NoSaveTag>(id)) {
                continue;
            }

            BuildViewsImpl(scene, id, p_out_views, p_is_opengl);
            break;
        }
    } else {
        BuildViewsImpl(scene, m_camera, p_out_views, p_is_opengl);
    }
}

// @TODO: rename this to DrawEditor
void SceneEditor::DrawMainView(const CameraComponent&) {
    // @TODO: fix this as well
    const CameraComponent* p_camera = GetScene()->GetComponent<CameraComponent>(m_camera);
    DEV_ASSERT(p_camera);

    ViewerTab::DrawMainView(*p_camera);

    const Matrix4x4f& view_matrix = p_camera->GetViewMatrix();
    const Matrix4x4f& proj_matrix = p_camera->GetProjectionMatrix();
    const Matrix4x4f& proj_view = p_camera->GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    Scene& scene = *m_document->m_scene.get();
    ecs::Entity id = GetSelectedEntity();
    TransformComponent* transform_component = scene.GetComponent<TransformComponent>(id);

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
                m_document->RequestMove(id, before, after, true);
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

        const float size = 120.f;
        const auto& min = m_viewer.GetCanvasMin();
        ImGuizmo::ViewManipulate((float*)&view_matrix[0].x,
                                 10.0f,
                                 ImVec2(min.x, min.y),
                                 ImVec2(size, size),
                                 IM_COL32(64, 64, 64, 96));
    }
}

bool SceneEditor::HandleInput(const InputEvent* p_input_event) {
    // change gizmo state
    if (auto e = dynamic_cast<const InputEventKey*>(p_input_event); e) {
        if (e->IsPressed() && !e->IsModiferPressed()) {
            bool handled = true;
            switch (e->GetKey()) {
                case KeyCode::KEY_Z: {
                    m_state = GizmoAction::Translate;
                } break;
                case KeyCode::KEY_X: {
                    m_state = GizmoAction::Rotate;
                } break;
                case KeyCode::KEY_C: {
                    m_state = GizmoAction::Scale;
                } break;
                default:
                    handled = false;
                    break;
            }
            return handled;
        }
    }

    // select
    if (auto e = dynamic_cast<const InputEventMouse*>(p_input_event); e) {
        if (e->IsButtonPressed(MouseButton::RIGHT)) {
            Vector2f clicked = e->GetPos();
            Select(clicked);
            return true;
        }
    }

    return false;
}

const std::vector<const ToolBarButtonDesc*> SceneEditor::GetToolBarButtons() const {
    return { &m_play_button };
}

void SceneEditor::Select(const Vector2f& p_cursor) {
    if (auto res = m_viewer.CursorToNDC(p_cursor); res.is_some()) {
        Vector2f ndc_2 = res.unwrap_unchecked();
        Vector4f ndc{ ndc_2.x, ndc_2.y, 1.0f, 1.0f };

        DEV_ASSERT(0);
        CameraComponent cam;

        const Matrix4x4f inv_pv = glm::inverse(cam.GetProjectionViewMatrix());

        const Vector3f ray_start = Vector3f::Zero;
        // const Vector3f ray_start = m_camera_transform.GetTranslation();
        const Vector3f direction = normalize(Vector3f((inv_pv * ndc).xyz));
        const Vector3f ray_end = ray_start + direction * cam.GetFar();
        Ray ray(ray_start, ray_end);

        const auto result = GetScene()->Intersects(ray);
        SelectEntity(result.entity);
    }
}

}  // namespace cave
