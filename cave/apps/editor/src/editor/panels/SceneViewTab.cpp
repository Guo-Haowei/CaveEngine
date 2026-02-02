#include "SceneViewTab.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>

#include "cave/runtime/framework/IApplication.h"

#include "engine/private/core/debugger/DebugIdAllocator.h"

#include "editor/edit/EditTransformCmd.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"
#include "editor/services/SelectionService.h"

// @TODO: refactor
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/runtime/scene/EntityFactory.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/render/render_device/RenderDevice.h"

#include "editor/document/SceneDocument.h"
#include "editor/EditorState.h"
#include "editor/utility/ImGuizmo.h"
#include "editor/viewer/Viewer.h"

#include "editor/EditorDvars.h"

namespace cave {

using math::Matrix4x4f;
using math::Vector2f;
using math::Vector3f;

SceneViewTab::SceneViewTab(EditorState& p_editor,
                           DocId p_doc_id,
                           SceneId p_preview_scene_id,
                           ViewDimension p_dimension)
    : Tab(p_editor, p_doc_id)
    , m_debug_id(MakeDebugId(this))
    , m_dim(p_dimension)
    , m_preview_scene(p_preview_scene_id)
    , m_button_displays{ ICON_FA_PLAY, ICON_FA_PAUSE }
    , m_button_tooltips{ "Run Project", "Pause Project" } {

    m_image_padding = Vector2f(10, 30);

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

// @TODO: game view tab
void SceneViewTab::BuildViewsImpl(SceneId p_scene_id,
                                  std::vector<render::ViewDesc>& p_out_views) {
    using namespace render;
    ViewDesc scene_view;
    if (m_editor.IsPlaying()) {
        scene_view.scene_id = m_editor.GetRuntimeHost().GetSceneId();
        scene_view.camera_source = CameraSource::MainCamera();
    } else {
        scene_view.scene_id = p_scene_id;
        scene_view.camera_source = CameraSource::Editor(m_camera);

        SelectionKey key = m_editor.SelectionService().Primary(m_doc_id);
        if (key.scene == p_scene_id && key.entity.IsValid()) {
            scene_view.highlight.entities.insert(key.entity);
        }
    }
    p_out_views.push_back(scene_view);
}

void SceneViewTab::OnCreate() {
    math::Vector2i frame_size = DVAR_GET_IVEC2(resolution);
    m_camera.SetWidth(frame_size.x);
    m_camera.SetHeight(frame_size.y);
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
}

void SceneViewTab::OnDestroy() {
    IApplication& app = m_editor.GetApp();
    app.GetSceneScheduler().Unregister(this);
}

void SceneViewTab::CollectSceneTicks(std::vector<SceneTickRequest>& p_out) {
    if (!m_editor.IsPlaying()) {
        p_out.push_back(SceneTickRequest{
            SceneTickMode::Editor,
            m_preview_scene,
        });
    }
}

void SceneViewTab::OnInputEvents(const std::vector<InputEvent>& p_events) {
    if (!IsHovered()) {
        return;
    }

    if (m_editor.IsPlaying()) {
        return;
    }

    bool skip_camera = false;
    for (const InputEvent& e : p_events) {
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
                    case Key::RMB: {
                        PickRequest req{};
                        req.tab_id = GetTabId();

                        req.cursor = Vector2f(e.x, e.y);
                        req.pos = Vector2f(m_view_rect.x, m_view_rect.y);
                        req.size = Vector2f(m_view_rect.w, m_view_rect.h);

                        m_editor.PickingService().Submit(std::move(req));
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
        m_camera_state = {};
        return;
    }

    const KeyState& st = m_editor.GetApp().GetInputSystem()->GetKeyState();
    if (st.AnyAltDown() || st.AnyCtrlDown() || st.AnyShiftDown()) {
        m_camera_state = {};
        return;
    }

    switch (m_dim) {
        case DIMENSION_2: {
            m_camera_state = CreateCameraInputState2D(p_events, st);
        } break;
        case DIMENSION_3: {
            m_camera_state = CreateCameraInputState3D(p_events, st);
        } break;
        default: {
            CRASH_NOW_MSG("invalid dimension");
        } break;
    }
}

void SceneViewTab::Tick(float p_dt) {
    Tab::Tick(p_dt);

    m_camera_state.move *= p_dt;
    m_camera_state.zoom_delta *= p_dt;
    m_camera_state.rotation *= p_dt;

    m_camera_controller->Update(m_camera_state);

    UpdateViewRect();
}

void SceneViewTab::DrawUIImpl() {
    DrawMainView();

    if (m_editor.IsPlaying()) return;
    if (IsFocused()) {
        DrawGizmo();
    }
}

void SceneViewTab::DrawMainView() {
    ImVec2 top_left(m_view_rect.x, m_view_rect.y);
    ImVec2 bottom_right(m_view_rect.Right(), m_view_rect.Bottom());

    // @TODO: add a dummy button
    const auto& gm = *m_editor.GetApp().GetRenderDevice();
    uint64_t handle = gm.GetFinalImage();
    // add image for drawing
    switch (gm.GetBackend()) {
        case Backend::D3D11:
        case Backend::D3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right);
        } break;
        case Backend::OPENGL: {
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            // if (gm.GetActiveRenderGraphName() == RenderGraphName::PATHTRACER) {
            //     uv_min = ImVec2(0, 0);
            //     uv_max = ImVec2(1, 1);
            // }
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right, uv_min, uv_max);
        } break;
        case Backend::VULKAN:
        case Backend::METAL: {
        } break;
        default:
            CRASH_NOW();
            break;
    }
}

void SceneViewTab::UpdateViewRect() {
    Vector2f top_left = m_top_left + m_image_padding;
    Vector2f size = m_size - m_image_padding;

    const float aspect = m_camera.GetAspect();
    if (aspect * size.y > size.x) {
        size.y = size.x / aspect;
    } else {
        size.x = size.y * aspect;
    }

    m_view_rect = {
        top_left.x,
        top_left.y,
        size.x,
        size.y,
    };
}

void SceneViewTab::DrawGizmo() {
    DEV_ASSERT(!m_camera.IsDirty());
    DocId doc_id = GetDocId();

    const Matrix4x4f& view_matrix = m_camera.GetViewMatrix();
    const Matrix4x4f& proj_matrix = m_camera.GetProjectionMatrix();
    const Matrix4x4f& proj_view = m_camera.GetProjectionViewMatrix();

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::BeginFrame();

    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(m_view_rect.x, m_view_rect.y, m_view_rect.w, m_view_rect.h);

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

Scene* SceneViewTab::GetResolvedScene() {
    return m_editor.GetApp().GetSceneRegistry()->Resolve(m_preview_scene);
}

CameraInputState SceneViewTab::CreateCameraInputState2D(const std::vector<InputEvent>& p_events, const KeyState& p_ks) {
    CameraInputState state{};

    const InputDeviceId id{ 0 };
    float dx = 0.0f;
    float dy = 0.0f;
    const bool mmb = p_ks.Down(id, Key::MMB);

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
            default:
                break;
        }
    }

    if (mmb) {
        state.move = math::Vector3f(dx, dy, 0.0f);
    }

    return state;
}

CameraInputState SceneViewTab::CreateCameraInputState3D(const std::vector<InputEvent>& p_events, const KeyState& p_st) {
    math::Vector2f rotation = math::Vector2f::Zero;

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

    state.move = math::Vector3f(dx, dy, dz);
    return state;
}

}  // namespace cave
