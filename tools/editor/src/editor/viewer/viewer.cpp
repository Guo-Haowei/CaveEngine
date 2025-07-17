#include "viewer.h"

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
#include "editor/viewer/tile_map_editor.h"
#include "editor/viewer/scene_editor.h"

namespace cave {

#define VIEWER_WINDOW_ID "###Viewer"

static constexpr float TOOL_BAR_OFFSET = 80.0f;

Viewer::Viewer(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
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

    m_tabs = {
        std::make_shared<ViewerTab>(m_editor, *this),
        std::make_shared<ViewerTab>(m_editor, *this),
        std::make_shared<ViewerTab>(m_editor, *this),
        std::make_shared<ViewerTab>(m_editor, *this),
    };
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

    ImGuiWindow* window = ImGui::FindWindowByName(GetTitle());
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

    ViewerTab* tool = GetActiveTab();
    const bool only_2d = tool ? tool->GetCameraPolicy() == ToolCameraPolicy::Only2D : false;

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

    auto active_tab = GetActiveTab();
    if (active_tab && active_tab->HandleInput(p_input_event)) {
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
    ViewerTab* tool = GetActiveTab();

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
    ViewerTab* tool = GetActiveTab();
    DEV_ASSERT(tool);

    UpdateFrameSize();

    if (m_focused) {
        UpdateCamera();
    }

    DrawGui(p_scene);

    // @TODO: should we update tool when it's not focused?
    tool->Update(p_scene);

    m_input_state.Reset();
}

void Viewer::RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close) {
    ImGui::OpenPopup("Save changes to");
    if (ImGui::BeginPopupModal("Save changes to")) {
        ImGui::Text("Save changes before closing?");
        if (ImGui::Button("Save")) {
            ImGui::CloseCurrentPopup();
            p_on_close(SaveDialogResponse::Save);
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard")) {
            ImGui::CloseCurrentPopup();
            p_on_close(SaveDialogResponse::Discard);
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            p_on_close(SaveDialogResponse::Cancel);
        }
        ImGui::EndPopup();
    }
}

void Viewer::HandleTabClose() {
    std::shared_ptr<ViewerTab> to_close;

    RequestSaveDialog([&](SaveDialogResponse p_response) {
        switch (p_response) {
            case Viewer::SaveDialogResponse::Save:
                // @TODO: save
                [[fallthrough]];
            case Viewer::SaveDialogResponse::Discard: {
                // remove the tab

                m_tabs.erase(
                    std::remove_if(m_tabs.begin(), m_tabs.end(), [&](std::shared_ptr<ViewerTab>& p_tab) {
                        if (p_tab->GetId() == m_close_request.unwrap()) {
                            to_close = p_tab;
                            return true;
                        }
                        return false;
                    }),
                    m_tabs.end());
            } break;
            case Viewer::SaveDialogResponse::Cancel:
                break;
        }

        m_close_request = Option<int>::None();
    });

    // @TODO: on tab leave, on destroy, etc
    if (to_close) {
        LOG_WARN("TODO: handle close ");
    }
}

void Viewer::OpenTab(AssetType p_type, const Guid& p_guid) {
    // check if tab already exists
    int tab_index = -1;
    for (size_t i = 0; i < m_tabs.size(); ++i) {
        if (m_tabs[i]->GetGuid() == p_guid) {
            tab_index = static_cast<int>(i);
            break;
        }
    }

    // if so, switch to it
    if (tab_index != -1) {
        SwitchTab(tab_index);
        return;
    }

    DEV_ASSERT(p_guid.IsValid());

    // else, create a new tab

    std::shared_ptr<ViewerTab> tab;

    switch (p_type) {
        case AssetType::Scene:
            tab.reset(new SceneEditor(m_editor, *this));
            break;
        case AssetType::TileMap:
            tab.reset(new TileMapEditor(m_editor, *this));
            break;
        default:
            LOG_WARN("Can't open tab {}", ToString(p_type));
            return;
    }

    tab->OnCreate(p_guid);
    m_tabs.emplace_back(std::move(tab));

    tab_index = static_cast<int>(m_tabs.size());

    SwitchTab(tab_index);
}

void Viewer::SwitchTab(int p_index) {
    DEV_ASSERT_INDEX(p_index, m_tabs.size());

    ViewerTab* new_tab = m_tabs[p_index].get();

    if (DEV_VERIFY(new_tab)) {
        ViewerTab* old_tab = GetActiveTab();

        if (old_tab) {
            old_tab->OnDeactivate();
        }

        m_active_tab = p_index;
        new_tab->OnActivate();

        LOG("Tool [{}] -> [{}]", old_tab ? old_tab->GetId() : -1, new_tab->GetId());
    }
}

void Viewer::UpdateInternal(Scene* p_scene) {
    // @TODO: tool bar policy
    DrawToolBar();

    if (!ImGui::BeginTabBar("MyTabs", ImGuiTabBarFlags_Reorderable)) {
        return;
    }
    const int tab_count = static_cast<int>(m_tabs.size());

    // go through all tabs
    for (int i = 0; i < tab_count; ++i) {
        ViewerTab* tab = m_tabs[i].get();

        bool tab_open = true;
        if (ImGui::BeginTabItem(tab->GetTitle().c_str(), &tab_open)) {
            if (m_active_tab != i) {
                SwitchTab(i);
            }

            tab->Draw(p_scene);
            ImGui::EndTabItem();
        }

        if (!tab_open) {
            m_close_request = tab->GetId();
        }
    }

    // handle close
    if (m_close_request.is_some()) {
        HandleTabClose();
    }

    if constexpr (false) {
        static bool tab_open = true;
        if (ImGui::BeginTabItem("mymymy", &tab_open)) {
            UpdateTab(p_scene);
            ImGui::EndTabItem();
        }
    }

    ImGui::EndTabBar();
}

// @NOTE: do not hold the pointer
ViewerTab* Viewer::GetActiveTab() {
    if (m_active_tab == -1 || m_active_tab >= static_cast<int>(m_tabs.size())) {
        return nullptr;
    }

    return m_tabs[m_active_tab].get();
}

}  // namespace cave
