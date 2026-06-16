#include "SceneViewTab.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/input/KeyState.h"

#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/view/ViewManager.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"

// @TODO: refactor
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorDvars.h"
#include "editor/EditorState.h"
#include "editor/utility/ImGuizmo.h"

namespace cave {

using namespace cave::literals;
using math::Matrix4x4f;
using math::Vec2f;
using math::Vec3f;
using math::Vec4f;

SceneViewTab::SceneViewTab(EditorState& editor,
                           DocId doc_id,
                           SceneId scene_id,
                           ViewDimension dim)
    : ViewTabBase(editor, doc_id, scene_id, dim)
    , debug_id_(MakeDebugId(this))
    , button_displays_{ ICON_FA_PLAY, ICON_FA_PAUSE }
    , button_tooltips_{ "Run Project", "Pause Project" } {

    play_button_ = {
        ICON_FA_PLAY,
        "Run Project",
        [this]() {
            m_editor.RequestModeSwitch();
            button_index_ = 1 - button_index_;
            play_button_.display = button_displays_[button_index_];
            play_button_.tooltip = button_tooltips_[button_index_];
        }
    };
}

// @TODO: game view tab
void SceneViewTab::submitView() {
    ViewTabBase::submitView(true);
}

void SceneViewTab::onCreate() {
    ViewTabBase::onCreate();

    editor_services_.picking().addConsumer(this);
}

void SceneViewTab::onDestroy() {
    ViewTabBase::onDestroy();

    editor_services_.picking().removeConsumer(this);
}

Option<PickData> SceneViewTab::getPickData(const Vec2f& point_os) {
    if (!isVisible()) return None();

    const ViewRecord* view = view_manager_.resolve(view_id_);
    if (!view->display_rect_os.Contains(point_os.x, point_os.y)) {
        return None();
    }

    return Some(PickData{
        .proj_view = camera_.GetProjectionViewMatrix(),
        .cursor_ndc = view->screenToNDC(point_os),
        .scene_id = preview_scene_id_,
        .doc_id = doc_id_,
    });
}

void SceneViewTab::onInputEvents(const InputFrame& input) {
    if (!isHovered()) {
        return;
    }

    if (m_editor.IsPlaying()) {
        return;
    }

    bool skip_camera = false;
    for (const InputEvent& e : input.events) {
        if (e.consumed) {
            continue;
        }

        switch (e.type) {
            case InputEventType::ButtonDown: {
                switch (static_cast<Key>(e.code)) {
                    case Key::Z: {
                        gizmo_action_ = GizmoAction::Translate;
                        e.consumed = true;
                    } break;
                    case Key::X: {
                        gizmo_action_ = GizmoAction::Rotate;
                        e.consumed = true;
                    } break;
                    case Key::C: {
                        gizmo_action_ = GizmoAction::Scale;
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

    if (skip_camera) {
        return;
    }

    const KeyState& st = app_services_.inputService().keyState();
    if (st.anyAltDown() || st.anyCtrlDown() || st.anyShiftDown()) {
        return;
    }

    camera_controller_->Update(input);
}

void SceneViewTab::drawUIImpl() {
    ViewRecord* view = view_manager_.resolve(view_id_);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    if (!m_editor.IsPlaying()) {
        drawGizmo(view->display_rect_os);
    }

    submitView();
}

// @TODO: instead of asking for image, provide an image to renderer
// @TODO: move this to gizmo
void SceneViewTab::drawGizmo(const math::FloatRect& rect) {
    DEV_ASSERT(!camera_.IsDirty());
    DocId doc_id = docId();

    const Matrix4x4f& view_matrix = camera_.GetViewMatrix();
    const Matrix4x4f& proj_matrix = camera_.GetProjectionMatrix();
    const Matrix4x4f& proj_view = camera_.GetProjectionViewMatrix();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rect.x, rect.y, rect.w, rect.h);

    SelectionKey selection = editor_services_.selection().Primary(doc_id_);
    ecs::Entity id = selection.entity;

    Scene* scene = getResolvedScene();
    TransformComponent* transform_component = scene->GetComponent<TransformComponent>(id);

    EditService& edit_service = editor_services_.edit();

    auto draw_gizmo = [&](ImGuizmo::OPERATION p_operation) {
        if (transform_component) {
            const Matrix4x4f before = transform_component->GetLocalMatrix();
            Matrix4x4f after = before;
            if (ImGuizmo::Manipulate(glm::value_ptr(view_matrix),
                                     glm::value_ptr(proj_matrix),
                                     p_operation,
                                     // ImGuizmo::LOCAL,
                                     ImGuizmo::WORLD,
                                     glm::value_ptr(after),
                                     nullptr, nullptr, nullptr, nullptr)) {

                Vec3f scale_1, scale_2;
                Vec3f pos_1, pos_2;
                Vec4f rot_1, rot_2;
                math::Decompose(before, scale_1, rot_1, pos_1);
                math::Decompose(after, scale_2, rot_2, pos_2);

                SceneRegistry& scene_reg = app_services_.sceneRegistry();
                if (p_operation & ImGuizmo::TRANSLATE) {
                    auto cmd = std::make_unique<ChangePropertyCmd>(
                        scene_reg,
                        id,
                        TransformComponent_Id,
                        "translation"_sid,
                        pos_1,
                        pos_2);
                    edit_service.submit(doc_id, std::move(cmd));
                } else if (p_operation & ImGuizmo::ROTATE) {
                    auto cmd = std::make_unique<ChangePropertyCmd>(
                        scene_reg,
                        id,
                        TransformComponent_Id,
                        "rotation"_sid,
                        rot_1,
                        rot_2);
                    edit_service.submit(doc_id, std::move(cmd));
                } else if (p_operation & ImGuizmo::SCALE) {
                    auto cmd = (std::make_unique<ChangePropertyCmd>(
                        scene_reg,
                        id,
                        TransformComponent_Id,
                        "scale"_sid,
                        scale_1,
                        scale_2));
                    edit_service.submit(doc_id, std::move(cmd));
                }
            }
        }
    };

    switch (gizmo_action_) {
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

        // const float size = 240.f;
        // ImGuizmo::ViewManipulate((float*)&view_matrix[0].x,
        //                          10.0f,
        //                          ImVec2(m_rect.x, m_rect.y),
        //                          ImVec2(size, size),
        //                          IM_COL32(64, 64, 64, 96));
    }
}

// const std::vector<const ToolBarButtonDesc*> SceneEditor::GetToolBarButtons() const {
//     return { &m_play_button };
// }

Scene* SceneViewTab::getResolvedScene() {
    return app_services_.sceneRegistry().resolve(preview_scene_id_);
}

}  // namespace cave
