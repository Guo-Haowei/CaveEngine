#include "SceneSelectTool.h"

#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/input/KeyCode.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/scene_view/SceneViewOverlay.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"

// TODO: fix
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/view/ViewManager.h"
#include "editor/EditorDvars.h"
#include "editor/utility/ImGuizmo.h"

namespace cave {

using namespace ::cave::math;
using ecs::Entity;

Option<PickData> SceneSelectTool::getPickData(const math::Vec2f& point_os) {
    const ViewRecord* view = m_ctx.engine_services.viewManager().resolve(m_ctx.view_id);
    if (!view->display_rect_os.Contains(point_os.x, point_os.y)) {
        return None();
    }

    return Some(PickData{
        .proj_view = m_ctx.camera.projectionViewMatrix(),
        .cursor_ndc = view->screenToNDC(point_os),
        .scene_id = m_ctx.scene_id,
        .doc_id = m_ctx.doc_id,
    });
}

void SceneSelectTool::onInputEvents(const InputFrame& input, const WindowState&) {
    bool skip_camera = false;
    for (const InputEvent& e : input.events) {
        if (e.consumed) {
            continue;
        }

        switch (e.type) {
            case InputEventType::ButtonDown: {
                switch (static_cast<Key>(e.code)) {
                    case Key::Z: {
                        m_gizmo_action = GizmoAction::Translate;
                        e.consumed = true;
                    } break;
                    case Key::X: {
                        m_gizmo_action = GizmoAction::Rotate;
                        e.consumed = true;
                    } break;
                    case Key::C: {
                        m_gizmo_action = GizmoAction::Scale;
                        e.consumed = true;
                    } break;
                    default:
                        break;
                }
                skip_camera = skip_camera || e.consumed;
            } break;
            default:
                break;
        }
    }
}

void SceneSelectTool::drawGizmo(const math::FloatRect& rect, bool ortho) {
    CameraComponent& camera = m_ctx.camera;
    DocId doc_id = m_ctx.doc_id;

    DEV_ASSERT(!camera.dirty());

    const Mat4f& view_matrix = camera.viewMatrix();
    const Mat4f& proj_matrix = camera.projectionMatrix();
    const Mat4f& proj_view = camera.projectionViewMatrix();

    ImGuizmo::SetOrthographic(ortho);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rect.x, rect.y, rect.w, rect.h);

    SelectionKey selection = m_ctx.editor_services.selection().primary(doc_id);
    ecs::Entity id = selection.entity;

    Scene* scene = getResolvedScene();
    DEV_ASSERT(scene);
    TransformComponent* transform_component = scene->component<TransformComponent>(id);

    EditService& edit_service = m_ctx.editor_services.edit();

    auto draw_gizmo = [&](ImGuizmo::OPERATION operation) {
        if (transform_component) {
            const Mat4f before = transform_component->localMatrix();
            Mat4f after = before;
            if (ImGuizmo::Manipulate(glm::value_ptr(view_matrix),
                                     glm::value_ptr(proj_matrix),
                                     operation,
                                     // ImGuizmo::LOCAL,
                                     ImGuizmo::WORLD,
                                     glm::value_ptr(after),
                                     nullptr, nullptr, nullptr, nullptr)) {

                Vec3f scale_1, scale_2;
                Vec3f pos_1, pos_2;
                Vec4f rot_1, rot_2;
                math::Decompose(before, scale_1, rot_1, pos_1);
                math::Decompose(after, scale_2, rot_2, pos_2);

                SceneRegistry& scene_reg = m_ctx.engine_services.sceneRegistry();
                if (operation & ImGuizmo::TRANSLATE) {
                    auto cmd = MakeOwner<ChangePropertyCmd>(
                        scene_reg,
                        ComponentPropertyTarget{
                            id,
                            TransformComponent_Id,
                            CAVE_SID("translation") },
                        pos_1,
                        pos_2);
                    edit_service.submit(doc_id, std::move(cmd));
                } else if (operation & ImGuizmo::ROTATE) {
                    auto cmd = MakeOwner<ChangePropertyCmd>(
                        scene_reg,
                        ComponentPropertyTarget{
                            id,
                            TransformComponent_Id,
                            CAVE_SID("rotation") },
                        rot_1,
                        rot_2);
                    edit_service.submit(doc_id, std::move(cmd));
                } else if (operation & ImGuizmo::SCALE) {
                    auto cmd = (MakeOwner<ChangePropertyCmd>(
                        scene_reg,
                        ComponentPropertyTarget{
                            id,
                            TransformComponent_Id,
                            CAVE_SID("scale") },
                        scale_1,
                        scale_2));
                    edit_service.submit(doc_id, std::move(cmd));
                }
            }
        }
    };

    switch (m_gizmo_action) {
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
    const bool show_editor = DVAR_GET_BOOL(show_editor);
    if (show_editor) {
        ImGuizmo::DrawAxes(proj_view);
    }
}

void SceneSelectTool::draw(const math::FloatRect& rect) {
    auto selection = m_ctx.editor_services.selection().primary(m_ctx.doc_id);

    if (Scene* scene = m_ctx.engine_services.sceneRegistry().resolve(selection.scene)) {
        SceneViewOverlay overlay;
        overlay.drawSelectionHighlight(m_ctx.engine_services.canvas(),
                                       m_ctx.view_id,
                                       *scene,
                                       selection.entity);
    }

    drawGizmo(rect, m_ctx.camera.isOrtho());
}

}  // namespace cave