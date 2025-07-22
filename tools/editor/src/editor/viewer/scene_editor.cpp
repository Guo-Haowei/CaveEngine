#include "scene_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "engine/runtime/asset_registry.h"
#include "engine/scene/entity_factory.h"

#include "editor/editor_command.h"
#include "editor/editor_layer.h"
#include "editor/editor_scene_manager.h"
#include "editor/viewer/viewer.h"
#include "editor/utility/imguizmo.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"
#include "editor/document/document.h"

namespace cave {

class SceneDocument : public Document {
public:
    SceneDocument(const Guid& p_guid)
        : Document(p_guid) {
        m_scene = m_handle.Wait<Scene>();
    }

private:
    std::shared_ptr<Scene> m_scene;

    friend class SceneEditor;
};

SceneEditor::SceneEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {
    ViewerTab::CreateDefaultCamera3D(m_cameras[0]);
    ViewerTab::CreateDefaultCamera2D(m_cameras[1]);
}

Document& SceneEditor::GetDocument() const {
    return *m_document.get();
}

void SceneEditor::OnCreate(const Guid& p_guid) {
    ViewerTab::OnCreate(p_guid);

    m_document = std::make_shared<SceneDocument>(p_guid);

    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);

    auto handle = AssetRegistry::GetSingleton().FindByGuid<Scene>(p_guid).unwrap();
}

void SceneEditor::OnDestroy() {
}

void SceneEditor::OnActivate() {
    auto scene_manager = static_cast<EditorSceneManager*>(m_editor.GetApplication()->GetSceneManager());
    DEV_ASSERT(scene_manager);
    scene_manager->SetTmpScene(m_document->m_scene);
}

void SceneEditor::DrawMainView(const CameraComponent& p_camera) {
    ViewerTab::DrawMainView(p_camera);

    const Matrix4x4f& view_matrix = p_camera.GetViewMatrix();
    const Matrix4x4f& proj_matrix = p_camera.GetProjectionMatrix();
    const Matrix4x4f& proj_view = p_camera.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer.GetCanvasMin();
    const Vector2f& canvas_size = m_viewer.GetCanvasSize();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    Scene& scene = *m_document->m_scene.get();
    ecs::Entity id = m_editor.GetSelectedEntity();
    TransformComponent* transform_component = scene.GetComponent<TransformComponent>(id);

#if 0
    if (transform_component) {
        LightComponent* light = scene.GetComponent<LightComponent>(id);
        if (light && light->GetType() == LIGHT_TYPE_INFINITE) {
            const auto& matrix = transform_component->GetWorldMatrix();
            ImGuizmo::DrawCone(proj_view, matrix);
        }
    }
#endif

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

                auto command = std::make_shared<EntityTransformCommand>(scene, id, before, after);
                m_editor.BufferCommand(command);
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

    // @TODO: move show_editor as viewer attribute
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

void SceneEditor::DrawAssetInspector() {
    // @TODO:
}

const CameraComponent& SceneEditor::GetActiveCameraInternal() const {
    return m_cameras[m_camera_idx];
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
            LOG_ERROR("TODO: implement select");
            // Vector2f clicked = e->GetPos();
            // m_viewer.GetInputState().ndc = m_viewer.CursorToNDC(clicked);
            return true;
        }
    }

    return false;
}

const std::vector<ToolBarButtonDesc>& SceneEditor::GetToolBarButtons() const {
    auto app = m_editor.GetApplication();
    auto app_state = app->GetState();

    static std::vector<ToolBarButtonDesc> s_buttons = {
        { ICON_FA_PLAY, "Run Project",
          [&]() { app->SetState(Application::State::BEGIN_SIM); },
          [&]() { return app_state != Application::State::SIM; } },
        { ICON_FA_PAUSE,
          "Pause Running Project",
          [&]() { app->SetState(Application::State::END_SIM); },
          [&]() { return app_state != Application::State::EDITING; } },
        { ICON_FA_CAMERA_ROTATE, "Toggle 2D/3D view",
          [&]() {
              m_camera_idx ^= 1;
          } },
    };

    return s_buttons;
}

#if 0
    void Process(Scene& p_scene, const CameraComponent& p_camera) override {
        if (!m_viewer.m_focused || !m_viewer.m_input_state.ndc) {
            return;
        }

        Vector2f clicked = *m_viewer.m_input_state.ndc;

        const Matrix4x4f inversed_projection_view = glm::inverse(p_camera.GetProjectionViewMatrix());

        const Vector3f ray_start = p_camera.GetPosition();
        const Vector3f direction = normalize(Vector3f((inversed_projection_view * Vector4f(clicked, 1.0f, 1.0f)).xyz));
        const Vector3f ray_end = ray_start + direction * p_camera.GetFar();
        Ray ray(ray_start, ray_end);

        const auto result = p_scene.Intersects(ray);

        m_viewer.m_editor.SelectEntity(result.entity);
    }

#endif

}  // namespace cave
