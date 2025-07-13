#include "editor_tool.h"

#include "engine/scene/scene.h"
#include "editor/editor_layer.h"
#include "editor/panels/viewer.h"
#include "editor/utility/imguizmo.h"

// @TODO: refactor
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"

namespace my {

void EditorTool::Update(Scene* p_scene) {
    const auto& cam = m_viewer->GetActiveCamera();
    const Matrix4x4f& view_matrix = cam.GetViewMatrix();
    const Matrix4x4f& proj_matrix = cam.GetProjectionMatrix();
    const Matrix4x4f& proj_view = cam.GetProjectionViewMatrix();

    const Vector2f& canvas_min = m_viewer->GetCanvasMin();
    const Vector2f& canvas_size = m_viewer->GetCanvasSize();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(canvas_min.x, canvas_min.y, canvas_size.x, canvas_size.y);

    bool show_editor = DVAR_GET_BOOL(show_editor);
    if (show_editor) {
        Matrix4x4f identity(1.0f);
        ImGuizmo::DrawGrid(proj_view, identity, 10.0f, ImGuizmo::GridPlane::XZ);
    }

    if (p_scene) {
        ecs::Entity id = m_editor.GetSelectedEntity();
        TransformComponent* transform_component = p_scene->GetComponent<TransformComponent>(id);
        if (transform_component) {
            LightComponent* light = p_scene->GetComponent<LightComponent>(id);
            if (light && light->GetType() == LIGHT_TYPE_INFINITE) {
                const auto& matrix = transform_component->GetWorldMatrix();
                ImGuizmo::DrawCone(proj_view, matrix);
            }
        }

        auto draw_gizmo = [&](ImGuizmo::OPERATION p_operation, GizmoAction p_action) {
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

                    auto command = std::make_shared<EntityTransformCommand>(p_action, *p_scene, id, before, after);
                    m_editor.BufferCommand(command);
                }
            }
        };

        // @TODO: fix
        switch (m_state) {
            case GizmoAction::Translate:
                draw_gizmo(ImGuizmo::TRANSLATE, m_state);
                break;
            case GizmoAction::Rotate:
                draw_gizmo(ImGuizmo::ROTATE, m_state);
                break;
            case GizmoAction::Scale:
                draw_gizmo(ImGuizmo::SCALE, m_state);
                break;
            default:
                break;
        }
    }

    if (show_editor) {
        const float size = 120.f;
        const auto& min = m_viewer->GetCanvasMin();
        ImGuizmo::ViewManipulate((float*)&view_matrix[0].x,
                                 10.0f,
                                 ImVec2(min.x, min.y),
                                 ImVec2(size, size),
                                 IM_COL32(64, 64, 64, 96));
    }
}

bool EditorTool::HandleInput(const std::shared_ptr<InputEvent>& p_input_event) {
    // change gizmo state
    InputEvent* event = p_input_event.get();
    if (auto e = dynamic_cast<InputEventKey*>(event); e) {
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
    if (auto e = dynamic_cast<InputEventMouse*>(event); e) {
        if (e->IsButtonPressed(MouseButton::RIGHT)) {
            Vector2f clicked = e->GetPos();
            m_viewer->GetInputState().ndc = m_viewer->CursorToNDC(clicked);
            return true;
        }
    }

    return false;
}

void EditorTool::OnEnter() {
}

void EditorTool::OnExit() {
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

}  // namespace my
