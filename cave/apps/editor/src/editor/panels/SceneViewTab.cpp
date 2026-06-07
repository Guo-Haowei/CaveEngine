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

SceneViewTab::SceneViewTab(EditorState& p_editor,
                           DocId p_doc_id,
                           SceneId p_preview_scene_id,
                           ViewDimension p_dimension)
    : Tab(p_editor, p_doc_id)
    , m_debug_id(MakeDebugId(this))
    , m_view_manager(*p_editor.GetApp().GetViewManager())
    , m_dim(p_dimension)
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
        m_texture = m_editor.GetApp().GetRenderDevice()->CreateTexture(
            desc,
            PointClampSampler());
    }
}

// @TODO: game view tab
void SceneViewTab::SubmitView() {
    using namespace render;
    ViewDesc view;
    view.view_id = m_view_id;
    view.viewport_px = { 0, 0, kTextureWidth, kTextureHeight };
    if (m_editor.IsPlaying()) {
        view.scene_id = m_editor.PIE().GetPIESceneId();
        view.camera_source = CameraSource::FirstCamera();
    } else {
        view.scene_id = m_preview_scene;
        view.camera_source = CameraSource::External(m_camera);

        SelectionKey key = m_editor.SelectionService().Primary(m_doc_id);
        if (key.scene == m_preview_scene && key.entity.IsValid()) {
            view.highlight.entities.insert(key.entity);
        }
    }
    view.output = m_texture;
    m_editor.GetApp().GetViewManager()->Submit(view);
}

void SceneViewTab::OnCreate() {
    m_camera.SetAspect((float)kTextureWidth / (float)kTextureHeight);
    m_camera.SetDirty();
    switch (m_dim) {
        case DIMENSION_2: {
            m_camera.SetProjection(ProjectionType::Orthographic);
            m_camera_controller = std::make_unique<CameraController2DEditor>(m_camera, m_camera_transform);
        } break;
        case DIMENSION_3: {
            m_camera_transform.Translate(Vector3f(0, 4, 8));
            m_camera_controller = std::make_unique<CameraControllerFPS>(m_camera, m_camera_transform);
        } break;
    }

    m_camera_transform.UpdateTransform();
    m_camera.Update(m_camera_transform.GetWorldMatrix());

    IApplication& app = m_editor.GetApp();

    app.GetSceneScheduler().Register(this);
    m_editor.PickingService().Register(this);

    m_view_id = app.GetViewManager()->CreateView(
        "SceneView",
        { 0, 0, kTextureWidth, kTextureHeight });
}

void SceneViewTab::OnDestroy() {
    IApplication& app = m_editor.GetApp();

    app.GetViewManager()->DestroyView(m_view_id);
    m_editor.PickingService().Register(this);
    app.GetSceneScheduler().Unregister(this);
}

Option<PickData> SceneViewTab::GetPickData(const math::Vector2f& p_pos_screen) {
    if (!IsVisible()) return None();

    const ViewRecord* view = m_view_manager.Resolve(m_view_id);
    if (!view->display_rect_os.Contains(p_pos_screen.x, p_pos_screen.y)) {
        return None();
    }

    return Some(PickData{
        .proj_view = m_camera.GetProjectionViewMatrix(),
        .cursor_ndc = view->screenToNDC(p_pos_screen),
        .scene_id = m_preview_scene,
        .doc_id = m_doc_id,
    });
}

void SceneViewTab::CollectSceneTicks(std::vector<SceneTickRequest>& p_out) {
    if (!m_editor.IsPlaying()) {
        p_out.push_back(SceneTickRequest{
            SceneTickMode::Editor,
            m_preview_scene,
        });
    }
}

void SceneViewTab::OnInputEvents(const InputFrame& p_input) {
    if (!IsHovered()) {
        return;
    }

    if (m_editor.IsPlaying()) {
        return;
    }

    bool skip_camera = false;
    for (const InputEvent& e : p_input.events) {
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

    const KeyState& st = m_editor.GetApp().InputService().keyState();
    if (st.anyAltDown() || st.anyCtrlDown() || st.anyShiftDown()) {
        return;
    }

    m_camera_controller->Update(p_input);
}

void SceneViewTab::DrawUIImpl() {
    ViewRecord* view = m_view_manager.Resolve(m_view_id);
    DEV_ASSERT(view);

    UpdateRect(view->display_rect_os);
    DrawMainView(view->display_rect_os);

    if (!m_editor.IsPlaying()) {
        DrawGizmo(view->display_rect_os);
    }

    SubmitView();
}

static void FitAspect(float p_aspect, float& p_width, float& p_height) {
    if (p_aspect * p_height > p_width) {
        p_height = p_width / p_aspect;
    } else {
        p_width = p_height * p_aspect;
    }
}

void SceneViewTab::UpdateRect(math::FloatRect& p_out_rect) {
    ImVec2 cursor_pos = ImGui::GetCursorPos();  // cursor to screen pos
    ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetWindowSize();
    {
        size.x -= 2 * cursor_pos.x;
        size.y -= 1.2f * cursor_pos.y;

        const float aspect = m_camera.GetAspect();
        FitAspect(aspect, size.x, size.y);
    }

    p_out_rect = math::FloatRect::FromMinMax(
        cursor_screen_pos.x,
        cursor_screen_pos.y,
        cursor_screen_pos.x + size.x,
        cursor_screen_pos.y + size.y);
}

// @TODO: instead of asking for image, provide an image to renderer
void SceneViewTab::DrawMainView(const math::FloatRect& p_rect) {
    const ImVec2 min{ p_rect.x, p_rect.y };
    const ImVec2 max{ p_rect.Right(), p_rect.Bottom() };

    // @TODO: move it somewhere else
    uint64_t handle = m_texture->GetHandle();
    // add image for drawing
    switch (m_editor.GetApp().GetBackend()) {
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
    ImGui::Dummy({ p_rect.w, p_rect.h });
    // ImGui::InvisibleButton("###DropTarget", size);
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CAVE/Asset")) {
        }
        ImGui::EndDragDropTarget();
    }
}

// @TODO: move this to gizmo
void SceneViewTab::DrawGizmo(const math::FloatRect& p_rect) {
    DEV_ASSERT(!m_camera.IsDirty());
    DocId doc_id = GetDocId();

    const Matrix4x4f& view_matrix = m_camera.GetViewMatrix();
    const Matrix4x4f& proj_matrix = m_camera.GetProjectionMatrix();
    const Matrix4x4f& proj_view = m_camera.GetProjectionViewMatrix();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(p_rect.x, p_rect.y, p_rect.w, p_rect.h);

    SelectionKey selection = m_editor.SelectionService().Primary(m_doc_id);
    ecs::Entity id = selection.entity;

    Scene* scene = GetResolvedScene();
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

                SceneRegistry& scene_reg = *m_editor.GetApp().GetSceneRegistry();
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

Scene* SceneViewTab::GetResolvedScene() {
    return m_editor.GetApp().GetSceneRegistry()->Resolve(m_preview_scene);
}

}  // namespace cave
