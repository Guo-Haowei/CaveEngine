#include "SceneViewTab.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/input/KeyState.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/assets/SceneAsset.h"
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

using namespace ::cave::literals;
using namespace ::cave::math;
using ecs::Entity;

SceneViewTab::SceneViewTab(EditorState& editor,
                           DocId doc_id,
                           SceneId scene_id,
                           ViewDimension dim)
    : ViewTabBase(editor, doc_id, scene_id, dim)
    , m_editor(editor)
    , m_debug_id(MakeDebugId(this))
    , m_button_displays{ ICON_FA_PLAY, ICON_FA_PAUSE }
    , m_button_tooltips{ "Run Project", "Pause Project" } {

    m_play_button = {
        ICON_FA_PLAY,
        "Run Project",
        [this]() {
            m_editor.requestModeSwitch();
            m_button_index = 1 - m_button_index;
            m_play_button.display = m_button_displays[m_button_index];
            m_play_button.tooltip = m_button_tooltips[m_button_index];
        }
    };
}

// @TODO: game view tab
void SceneViewTab::submitView() {
    ViewTabBase::submitView(true);
}

void SceneViewTab::onCreate() {
    ViewTabBase::onCreate();

    m_editor_services.picking().addConsumer(this);
}

void SceneViewTab::onDestroy() {
    ViewTabBase::onDestroy();

    m_editor_services.picking().removeConsumer(this);
}

Option<PickData> SceneViewTab::getPickData(const Vec2f& point_os) {
    if (!isVisible()) return None();

    const ViewRecord* view = m_view_manager.resolve(m_view_id);
    if (!view->display_rect_os.Contains(point_os.x, point_os.y)) {
        return None();
    }

    return Some(PickData{
        .proj_view = m_camera.projectionViewMatrix(),
        .cursor_ndc = view->screenToNDC(point_os),
        .scene_id = m_preview_scene_id,
        .doc_id = m_doc_id,
    });
}

void SceneViewTab::onInputEvents(const InputFrame& input) {
    if (!isHovered()) {
        return;
    }

    if (m_editor.isPlaying()) {
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

    if (skip_camera) {
        return;
    }

    const KeyState& st = m_engine_services.inputService().keyState();
    if (st.anyAltDown() || st.anyCtrlDown() || st.anyShiftDown()) {
        return;
    }

    m_camera_controller->update(input);
}

void SceneViewTab::drawUIImpl() {
    ViewRecord* view = m_view_manager.resolve(m_view_id);
    DEV_ASSERT(view);

    updateRect(view->display_rect_os);
    drawMainView(view->display_rect_os);

    if (!m_editor.isPlaying()) {
        drawGizmo(view->display_rect_os);
    }

    submitView();
}

// @TODO: instead of asking for image, provide an image to renderer
// @TODO: move this to gizmo
void SceneViewTab::drawGizmo(const math::FloatRect& rect) {
    DEV_ASSERT(!m_camera.dirty());
    DocId doc_id = docId();

    const Mat4f& view_matrix = m_camera.viewMatrix();
    const Mat4f& proj_matrix = m_camera.projectionMatrix();
    const Mat4f& proj_view = m_camera.projectionViewMatrix();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(rect.x, rect.y, rect.w, rect.h);

    SelectionKey selection = m_editor_services.selection().Primary(m_doc_id);
    ecs::Entity id = selection.entity;

    Scene* scene = getResolvedScene();
    TransformComponent* transform_component = scene->component<TransformComponent>(id);

    EditService& edit_service = m_editor_services.edit();

    auto draw_gizmo = [&](ImGuizmo::OPERATION p_operation) {
        if (transform_component) {
            const Mat4f before = transform_component->localMatrix();
            Mat4f after = before;
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

                SceneRegistry& scene_reg = m_engine_services.sceneRegistry();
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

void SceneViewTab::onAssetDropped(AssetHandle&& handle) {
    IAsset* asset = handle.get();
    switch (asset->type()) {
        case AssetType::Scene: {
            Scene* scene = getResolvedScene();
            m_editor_services.edit().submit(m_doc_id, [&](SceneCommandWriter& writer) {
                Entity prefb = writer.prefabObject("prefab", handle.guid());
                writer.attachChild(prefb, scene->root());
            });
        } break;
            // @TODO: prefab
        default:
            break;
    }

    const AssetMetaData* meta = handle.meta();
    LOG_ERROR(LogChannel::Asset, "asset {} not accepted", meta->name);
}

Scene* SceneViewTab::getResolvedScene() {
    return m_engine_services.sceneRegistry().resolve(m_preview_scene_id);
}

}  // namespace cave
