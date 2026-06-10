#include "SceneViewTab.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/input/KeyState.h"

#include "engine/private/runtime/framework/IRenderDevice.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/view/ViewManager.h"

#include "editor/edit/ChangePropertyCmd.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"

// @TODO: refactor
#include "engine/private/core/math/Geomath.h"
#include "engine/private/renderer/gpu_resource.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/sampler.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/document/SceneDocument.h"
#include "editor/EditorDvars.h"
#include "editor/EditorState.h"
#include "editor/utility/ImGuizmo.h"

namespace cave {

using namespace cave::literals;
using math::Matrix4x4f;
using math::Vector2f;
using math::Vector3f;
using math::Vector4f;
using rhi::Backend;

#if 1
static constexpr uint32_t kTextureWidth = 1920;
static constexpr uint32_t kTextureHeight = 1080;
#else
static constexpr uint32_t kTextureWidth = 640;
static constexpr uint32_t kTextureHeight = 480;
#endif

SceneViewTab::SceneViewTab(EditorState& editor,
                           DocId doc_id,
                           SceneId scene_id,
                           ViewDimension dim)
    : Tab(editor, doc_id)
    , view_manager_(editor.app().services().viewManager())
    , dim_(dim)
    , debug_id_(MakeDebugId(this))
    , preview_scene_id_(scene_id)
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

    // @TODO: move it to somewhere else
    {
        GpuTextureDesc desc{
            .type = AttachmentType::COLOR_2D,
            .dimension = Dimension::TEXTURE_2D,
            .width = kTextureWidth,
            .height = kTextureHeight,
            .depth = 1,
            .mipLevels = 0,
            .arraySize = 1,
            .format = PixelFormat::R16G16B16A16_FLOAT,
            .bindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE,
            .miscFlags = RESOURCE_MISC_NONE,
        };
        texture_ = m_editor.app().GetRenderDevice()->CreateTexture(
            desc,
            PointClampSampler());
    }
}

// @TODO: game view tab
void SceneViewTab::submitView() {
    using namespace render;
    ViewDesc view;
    view.view_id = view_id_;
    view.viewport_px = { 0, 0, kTextureWidth, kTextureHeight };
    if (m_editor.IsPlaying()) {
        view.scene_id = m_editor.PIE().getPIESceneId();
        view.camera_source = CameraSource::FirstCamera();
    } else {
        view.scene_id = preview_scene_id_;
        view.camera_source = CameraSource::External(camera_);

        SelectionKey key = m_editor.SelectionService().Primary(doc_id_);
        if (key.scene == preview_scene_id_ && key.entity.IsValid()) {
            view.highlight.entities.insert(key.entity);
        }
    }
    view.output = texture_;
    view_manager_.submit(view);
}

void SceneViewTab::onCreate() {
    camera_.SetAspect((float)kTextureWidth / (float)kTextureHeight);
    camera_.SetDirty();
    switch (dim_) {
        case ViewDimension::Dim2: {
            camera_.SetProjection(ProjectionType::Orthographic);
            camera_transform_.Translate(Vector3f(0, 0, 4));
            camera_controller_ = std::make_unique<CameraController2DEditor>(camera_, camera_transform_);
        } break;
        case ViewDimension::Dim3: {
            camera_transform_.Translate(Vector3f(0, 4, 8));
            camera_controller_ = std::make_unique<CameraControllerFPS>(camera_, camera_transform_);
        } break;
    }

    camera_transform_.UpdateTransform();
    camera_.Update(camera_transform_.GetWorldMatrix());

    IApplication& app = m_editor.app();

    app.services().sceneScheduler().Register(this);
    m_editor.PickingService().Register(this);

    view_id_ = view_manager_.createView(
        "SceneView",
        { 0, 0, kTextureWidth, kTextureHeight });
}

void SceneViewTab::onDestroy() {
    IApplication& app = m_editor.app();

    view_manager_.destroyView(view_id_);
    m_editor.PickingService().Register(this);
    app.services().sceneScheduler().Unregister(this);
}

Option<PickData> SceneViewTab::GetPickData(const math::Vector2f& pointer_os) {
    if (!IsVisible()) return None();

    const ViewRecord* view = view_manager_.resolve(view_id_);
    if (!view->display_rect_os.Contains(pointer_os.x, pointer_os.y)) {
        return None();
    }

    return Some(PickData{
        .proj_view = camera_.GetProjectionViewMatrix(),
        .cursor_ndc = view->screenToNDC(pointer_os),
        .scene_id = preview_scene_id_,
        .doc_id = doc_id_,
    });
}

void SceneViewTab::CollectSceneTicks(std::vector<SceneTickRequest>& out_requests) {
    if (!m_editor.IsPlaying()) {
        out_requests.push_back(SceneTickRequest{
            SceneTickMode::Editor,
            preview_scene_id_,
        });
    }
}

void SceneViewTab::onInputEvents(const InputFrame& input) {
    if (!IsHovered()) {
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

    const KeyState& st = services_.inputService().keyState();
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

// @TODO: refactor
static void fitAspect(float aspect, float& w, float& h) {
    if (aspect * h > w) {
        h = w / aspect;
    } else {
        w = h * aspect;
    }
}

void SceneViewTab::updateRect(math::FloatRect& out_rect) {
    ImVec2 cursor_pos = ImGui::GetCursorPos();  // cursor to screen pos
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetWindowSize();
    {
        size.x -= 2 * cursor_pos.x;
        size.y -= 1.2f * cursor_pos.y;

        const float aspect = camera_.GetAspect();
        fitAspect(aspect, size.x, size.y);
    }

    out_rect = math::FloatRect::FromMinMax(
        cursor_screen_pos.x,
        cursor_screen_pos.y,
        cursor_screen_pos.x + size.x,
        cursor_screen_pos.y + size.y);
}

// @TODO: instead of asking for image, provide an image to renderer
void SceneViewTab::drawMainView(const math::FloatRect& rect) {
    const ImVec2 min{ rect.x, rect.y };
    const ImVec2 max{ rect.Right(), rect.Bottom() };

    // @TODO: move it somewhere else
    uint64_t handle = texture_->GetHandle();
    // add image for drawing
    switch (m_editor.app().GetBackend()) {
        case Backend::Direct3D11:
        case Backend::Direct3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, min, max);
        } break;
        case Backend::OpenGL: {
            // @TODO: add p_flip
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, min, max, uv_min, uv_max);
        } break;
        case Backend::Vulkan:
        case Backend::Metal: {
        } break;
        default:
            CRASH_NOW();
            break;
    }

    // @TODO: drop target
    ImGui::Dummy({ rect.w, rect.h });
    // ImGui::InvisibleButton("###DropTarget", size);
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CAVE/Asset")) {
        }
        ImGui::EndDragDropTarget();
    }
}

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

    SelectionKey selection = m_editor.SelectionService().Primary(doc_id_);
    ecs::Entity id = selection.entity;

    Scene* scene = getResolvedScene();
    TransformComponent* transform_component = scene->GetComponent<TransformComponent>(id);

    EditService& edit_service = m_editor.EditService();

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

                Vector3f scale_1, scale_2;
                Vector3f pos_1, pos_2;
                Vector4f rot_1, rot_2;
                math::Decompose(before, scale_1, rot_1, pos_1);
                math::Decompose(after, scale_2, rot_2, pos_2);

                SceneRegistry& scene_reg = services_.sceneRegistry();
                if (p_operation & ImGuizmo::TRANSLATE) {
                    auto cmd = std::make_unique<ChangePropertyCmd>(
                        scene_reg,
                        id,
                        TransformComponent_Id,
                        "translation"_sid,
                        pos_1,
                        pos_2);
                    edit_service.Submit(doc_id, std::move(cmd));
                } else if (p_operation & ImGuizmo::ROTATE) {
                    auto cmd = std::make_unique<ChangePropertyCmd>(
                        scene_reg,
                        id,
                        TransformComponent_Id,
                        "rotation"_sid,
                        rot_1,
                        rot_2);
                    edit_service.Submit(doc_id, std::move(cmd));
                } else if (p_operation & ImGuizmo::SCALE) {
                    auto cmd = (std::make_unique<ChangePropertyCmd>(
                        scene_reg,
                        id,
                        TransformComponent_Id,
                        "scale"_sid,
                        scale_1,
                        scale_2));
                    edit_service.Submit(doc_id, std::move(cmd));
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
    return services_.sceneRegistry().resolve(preview_scene_id_);
}

}  // namespace cave
