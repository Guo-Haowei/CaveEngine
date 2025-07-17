#include "viewer_tab_manager.h"

#include <IconsFontAwesome/IconsFontAwesome6.h>
#include <imgui/imgui_internal.h>

#include "editor/editor_layer.h"
#include "editor/utility/imguizmo.h"
#include "editor/widget.h"
#include "engine/input/input_event.h"
#include "engine/math/ray.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/common_dvars.h"
#include "engine/runtime/display_manager.h"

namespace cave {

#define VIEWER_WINDOW_ID "###Viewer"

static constexpr float TOOL_BAR_OFFSET = 40.0f;

Viewer::Viewer(EditorLayer& p_editor)
    : EditorWindow("Viewer" VIEWER_WINDOW_ID, p_editor) {
    const auto res = DVAR_GET_IVEC2(resolution);
    {
        CameraComponent camera;
        camera.SetDimension(res.x, res.y);
        camera.SetNear(1.0f);
        camera.SetFar(1000.0f);
        camera.SetPosition(Vector3f(0, 4, 10));
        camera.SetDirty();
        camera.Update();

        m_controller.cameras[CAM3D] = camera;
    }
    {
        CameraComponent camera;
        camera.SetOrtho();
        camera.SetDimension(res.x, res.y);
        camera.SetNear(1.0f);
        camera.SetFar(1000.0f);
        camera.SetPosition(Vector3f(0, 0, 10));
        camera.SetDirty();
        camera.Update();

        m_controller.cameras[CAM2D] = camera;
    }
}

void Viewer::UpdateFrameSize() {
    Vector2i frame_size = DVAR_GET_IVEC2(resolution);
    int frame_width = frame_size.x;
    int frame_height = frame_size.y;
    const float ratio = (float)frame_width / frame_height;
    m_canvas_size.x = ImGui::GetWindowSize().x;
    m_canvas_size.y = ImGui::GetWindowSize().y;
    if (m_canvas_size.y * ratio > m_canvas_size.x) {
        m_canvas_size.y = m_canvas_size.x / ratio;
    } else {
        m_canvas_size.x = m_canvas_size.y * ratio;
    }

    ImGuiWindow* window = ImGui::FindWindowByName(m_name.c_str());
    DEV_ASSERT(window);
    m_canvas_min.x = window->ContentRegionRect.Min.x;
    m_canvas_min.y = TOOL_BAR_OFFSET + window->ContentRegionRect.Min.y;

    m_focused = ImGui::IsWindowHovered();
}

void Viewer::DrawGui(Scene*) {
    // @TODO: fix this
    const auto& gm = IGraphicsManager::GetSingleton();
    uint64_t handle = gm.GetFinalImage();
    // add image for drawing
    ImVec2 top_left(m_canvas_min.x, m_canvas_min.y);
    ImVec2 bottom_right(top_left.x + m_canvas_size.x, top_left.y + m_canvas_size.y);
    switch (gm.GetBackend()) {
        case Backend::D3D11:
        case Backend::D3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right);
        } break;
        case Backend::OPENGL: {
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            if (gm.GetActiveRenderGraphName() == RenderGraphName::PATHTRACER) {
                uv_min = ImVec2(0, 0);
                uv_max = ImVec2(1, 1);
            }
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

void Viewer::DrawToolBar() {
    struct ToolBarButtonDesc {
        const char* display{ nullptr };
        const char* tooltip{ nullptr };
        std::function<void()> func;
        std::function<bool()> enabledFunc;
    };

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    auto app = m_editor.GetApplication();
    auto app_state = app->GetState();

    ViewerTab* tool = m_editor.GetActiveTool();
    const bool only_2d = tool->GetCameraPolicy() == ToolCameraPolicy::Only2D;

    static const ToolBarButtonDesc s_buttons[] = {
        { ICON_FA_PLAY, "Run Project",
          [&]() { app->SetState(Application::State::BEGIN_SIM); },
          [&]() { return app_state != Application::State::SIM; } },
        { ICON_FA_PAUSE, "Pause Running Project",
          [&]() { app->SetState(Application::State::END_SIM); },
          [&]() { return app_state != Application::State::EDITING; } },
        { ICON_FA_HAND, "Enter gizmo mode",
          [&]() {
          } },
        { ICON_FA_CAMERA_ROTATE, "Toggle 2D/3D view",
          [&]() {
              m_controller.Toggle(only_2d);
          } },
        { ICON_FA_BRUSH, "TileMap editor mode",
          [&]() {
          } },
    };

    for (int i = 0; i < array_length(s_buttons); ++i) {
        const auto& desc = s_buttons[i];
        const bool enabled = desc.enabledFunc ? desc.enabledFunc() : true;

        if (i != 0) {
            ImGui::SameLine();
        }

        if (!enabled) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
        }

        if (ImGui::Button(desc.display) && enabled) {
            desc.func();
        }

        if (!enabled) {
            ImGui::PopStyleVar();
        }

        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text(desc.tooltip);
            ImGui::EndTooltip();
        }
    }

    // ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

HandleInputResult Viewer::HandleInput(std::shared_ptr<InputEvent> p_input_event) {
    if (!m_focused) {
        return HandleInputResult::NotHandled;
    }

    if (m_editor.GetActiveTool()->HandleInput(p_input_event)) {
        return HandleInputResult::Handled;
    }

    return HandleInputCamera(p_input_event);
}

std::optional<Vector2f> Viewer::CursorToNDC(Vector2f p_point) const {
    auto [window_x, window_y] = m_editor.GetApplication()->GetDisplayServer()->GetWindowPos();
    p_point.x = (p_point.x + window_x - m_canvas_min.x) / m_canvas_size.x;
    p_point.y = (p_point.y + window_y - m_canvas_min.y) / m_canvas_size.y;

    if (p_point.x >= 0.0f && p_point.x <= 1.0f && p_point.y >= 0.0f && p_point.y <= 1.0f) {
        p_point *= 2.0f;
        p_point -= 1.0f;
        p_point.y = -p_point.y;
        return p_point;
    }

    return std::nullopt;
}

HandleInputResult Viewer::HandleInputCamera(std::shared_ptr<InputEvent> p_input_event) {
    InputEvent* event = p_input_event.get();

    if (auto e = dynamic_cast<InputEventKey*>(event); e) {
        if (e->IsHolding() && !e->IsModiferPressed()) {
            HandleInputResult handled = HandleInputResult::Handled;
            switch (e->GetKey()) {
                case KeyCode::KEY_D:
                    ++m_input_state.dx;
                    break;
                case KeyCode::KEY_A:
                    --m_input_state.dx;
                    break;
                case KeyCode::KEY_E:
                    ++m_input_state.dy;
                    break;
                case KeyCode::KEY_Q:
                    --m_input_state.dy;
                    break;
                case KeyCode::KEY_W:
                    ++m_input_state.dz;
                    break;
                case KeyCode::KEY_S:
                    --m_input_state.dz;
                    break;
                default:
                    handled = HandleInputResult::NotHandled;
                    break;
            }
            return handled;
        }
    }

    if (auto e = dynamic_cast<InputEventMouseWheel*>(event); e) {
        if (!e->IsModiferPressed()) {
            m_input_state.scroll += 3.0f * e->GetWheelY();
            return HandleInputResult::Handled;
        }
    }

    if (auto e = dynamic_cast<InputEventMouseMove*>(event); e) {
        if (!e->IsModiferPressed() && e->IsButtonDown(MouseButton::MIDDLE)) {
            m_input_state.mouse_move += e->GetDelta();
            return HandleInputResult::Handled;
        }
    }

    return HandleInputResult::NotHandled;
}

void Viewer::UpdateCamera() {
    ViewerTab* tool = m_editor.GetActiveTool();

    CameraComponent& camera = GetActiveCamera();

    const float dt = m_editor.context.timestep;
    const auto& move = m_input_state.mouse_move;
    const auto& scroll = m_input_state.scroll;

    const bool only_2d = tool->GetCameraPolicy() == ToolCameraPolicy::Only2D;
    m_controller.Check(only_2d);

    // @TODO: still need to figure out a better way to do it
    switch (m_controller.current) {
        case CAM2D: {
            CameraInputState state{
                .move = dt * Vector3f(-move.x, move.y, 0.0f),
                .zoomDelta = -dt * scroll,
            };
            m_controller.controller_2d.Update(camera, state);
        } break;
        case CAM3D: {
            CameraInputState state{
                .move = dt * Vector3f(m_input_state.dx, m_input_state.dy, m_input_state.dz),
                .zoomDelta = dt * scroll,
                .rotation = dt * move,
            };
            m_controller.controller_3d.Update(camera, state);
        } break;
    }

    camera.Update();
}

void Viewer::UpdateTab(Scene* p_scene) {
    ViewerTab* tool = m_editor.GetActiveTool();
    DEV_ASSERT(tool);

    // update name
    m_name = std::format("{}" VIEWER_WINDOW_ID, tool->GetTile());

    UpdateFrameSize();

    if (m_focused) {
        UpdateCamera();
    }

    DrawGui(p_scene);

    // @TODO: should we update tool when it's not focused?
    tool->Update(p_scene);

    m_input_state.Reset();
}

void Viewer::UpdateInternal(Scene* p_scene) {
    // @TODO: tool bar policy
    DrawToolBar();

    if (ImGui::BeginTabBar("MyTabs", ImGuiTabBarFlags_Reorderable)) {
        {
            static bool tab_open = true;
            if (ImGui::BeginTabItem("mymymy", &tab_open)) {
                UpdateTab(p_scene);
                ImGui::EndTabItem();
            }
        }

        ImGui::EndTabBar();
    }
}

}  // namespace cave
