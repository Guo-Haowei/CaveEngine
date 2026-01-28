#include "SceneEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/runtime/framework/IApplication.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/EntityFactory.h"

#include "editor/document/document.h"
#include "editor/document/DocumentService.h"
#include "editor/document/SceneDocument.h"
#include "editor/EditorState.h"
#include "editor/utility/ImGuizmo.h"
#include "editor/viewer/Viewer.h"

// @TODO: refactor
#include "editor/EditorDvars.h"

namespace cave {

SceneEditor::SceneEditor(EditorState& p_editor, Viewer& p_viewer, ViewerTab::Dimension p_dimension)
    : ViewerTab(p_editor, p_viewer, p_dimension)
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

OldDocument& SceneEditor::GetDocument() const {
    return *((OldDocument*)nullptr);
}

SceneId SceneEditor::GetSceneId() const {
    IDocument* doc = m_editor.DocumentService().Resolve(m_doc_id);
    if (!doc) return {};
    return doc->GetPreviewScene();
}

void SceneEditor::OnCreateInternal(const Guid& p_guid) {
    m_doc_id = m_editor.DocumentService().OpenScene(p_guid);
}

void SceneEditor::OnDestroy() {
    m_editor.DocumentService().Close(m_doc_id);
}

void SceneEditor::OnActivateInternal() {
}

void SceneEditor::BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) {
    if (!m_active) {
        return;
    }

    BuildViewsImpl(GetSceneId(), m_camera, p_out_views, p_is_opengl);
}

// @TODO: rename this to DrawEditor
void SceneEditor::DrawMainView(const CameraComponent&) {
    Scene& scene = *GetResolvedScene();
    CameraComponent& camera = *scene.GetComponent<CameraComponent>(m_camera);

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
                // m_document->RequestMove(id, before, after, true);
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

#if 0
bool SceneEditor::HandleInput(const OldInputEvent* p_input_event) {
    unused(p_input_event);
    // change gizmo state
    if (auto e = dynamic_cast<const InputEventKey*>(p_input_event); e) {
        if (e->IsPressed() && !e->IsModiferPressed()) {
            bool handled = true;
            switch (e->GetKey()) {
                case Key::Z: {
                    m_state = GizmoAction::Translate;
                } break;
                case Key::X: {
                    m_state = GizmoAction::Rotate;
                } break;
                case Key::C: {
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
    DEV_ASSERT(0);

    return false;
}
#endif

const std::vector<const ToolBarButtonDesc*> SceneEditor::GetToolBarButtons() const {
    return { &m_play_button };
}

void SceneEditor::Select(const Vector2f& p_cursor) {
    unused(p_cursor);
    DEV_ASSERT(0);
#if 0
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
#endif
}

}  // namespace cave
