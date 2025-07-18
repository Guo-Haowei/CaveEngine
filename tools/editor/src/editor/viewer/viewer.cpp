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

Option<ViewerTab*> TabManager::FindTabById(const TabId& p_id) {
    auto it = m_tabs.find(p_id);
    if (it != m_tabs.end()) {
        return it->second.get();
    }
    return Option<ViewerTab*>::None();
}

Option<ViewerTab*> TabManager::FindTabByGuid(const Guid& p_guid) {
    for (const auto& [id, tab] : m_tabs) {
        if (tab->GetGuid() == p_guid) {
            return tab.get();
        }
    }
    return Option<ViewerTab*>::None();
}

Option<ViewerTab*> TabManager::GetActiveTab() {
    if (m_active_tab.is_none()) {
        return Option<ViewerTab*>::None();
    }
    return FindTabById(m_active_tab.unwrap());
}

void TabManager::SwitchTab(std::shared_ptr<ViewerTab>&& p_tab) {
    const auto& id = p_tab->GetId();
    auto [it, ok] = m_tabs.try_emplace(p_tab->GetId(), std::move(p_tab));
    DEV_ASSERT(ok);

    SwitchTab(id);
}

void TabManager::SwitchTab(const TabId& p_id) {
    if (m_active_tab == p_id) {
        return;
    }

    auto new_tab = FindTabById(p_id).unwrap();
    auto old_tab = GetActiveTab();

    if (old_tab.is_some()) {
        old_tab.unwrap()->OnDeactivate();
    }

    m_active_tab = p_id;
    m_focus_request = p_id;

    new_tab->OnActivate();

    LOG("Tool [{}] -> [{}]", old_tab.is_some() ? old_tab.unwrap()->GetTitle() : "(null)", new_tab->GetTitle());
}

void TabManager::HandleTabClose() {
    if (m_close_request.is_none()) {
        return;
    }

    std::shared_ptr<ViewerTab> to_close;

    RequestSaveDialog([&](SaveDialogResponse p_response) {
        switch (p_response) {
            case SaveDialogResponse::Save:
                // @TODO: save
                [[fallthrough]];
            case SaveDialogResponse::Discard: {
                // remove the tab
                auto it = m_tabs.find(m_close_request.unwrap());
                DEV_ASSERT(it != m_tabs.end());
                to_close = it->second;
                m_tabs.erase(it);
            } break;
            case SaveDialogResponse::Cancel:
                break;
        }

        m_close_request = Option<TabId>::None();
    });

    if (to_close) {
        to_close->OnDeactivate();
        to_close->OnDestroy();
    }
}

void TabManager::RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close) {
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

Viewer::Viewer(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
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

    // ViewerTab* tool = GetActiveTab();
    // const bool only_2d = tool ? tool->GetCameraPolicy() == ToolCameraPolicy::Only2D : false;

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
              // m_controller.Toggle(only_2d);
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
    // @TODO: cache input and redirect to tabs

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

// @TODO: move it inside?
void Viewer::UpdateTab(Scene* p_scene) {
    ViewerTab* tool = GetActiveTab();
    DEV_ASSERT(tool);

    if (m_focused) {
        tool->UpdateCamera();
    }

    DrawGui(p_scene);

    // @TODO: should we update tool when it's not focused?
    tool->Update(p_scene);
}

void Viewer::OpenTab(AssetType p_type, const Guid& p_guid) {
    // check if tab already exists
    auto cached_tab = m_tab_manager.FindTabByGuid(p_guid);

    if (cached_tab.is_some()) {
        m_tab_manager.SwitchTab(cached_tab.unwrap()->GetId());
        return;
    }

    DEV_ASSERT(!p_guid.IsNull());

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
    m_tab_manager.SwitchTab(std::move(tab));
}

void Viewer::UpdateInternal(Scene* p_scene) {
    // @TODO: tool bar policy
    DrawToolBar();

    UpdateFrameSize();

    int flag = 0;
#if 1
    flag |= ImGuiTabBarFlags_Reorderable;
#endif
    if (!ImGui::BeginTabBar("MyTabs", flag)) {
        return;
    }

    // go through all tabs
    // see if there's a focus request

    TabId focus_tab_id = m_tab_manager.GetFocusRequest().unwrap_or(TabId::Null());
    for (auto& [id, tab] : m_tab_manager.GetTabs()) {
        int flags = ImGuiTabItemFlags_UnsavedDocument;

        if (tab->GetId() == focus_tab_id) {
            flags |= ImGuiTabItemFlags_SetSelected;
        }

        bool tab_open = true;
        if (ImGui::BeginTabItem(tab->GetTitle().c_str(), &tab_open, flags)) {
            if (focus_tab_id == TabId::Null()) {
                m_tab_manager.SwitchTab(tab->GetId());
            }

            UpdateTab(p_scene);
            tab->Draw(p_scene);
            ImGui::EndTabItem();
        }

        if (!tab_open) {
            m_tab_manager.RequestClose(tab->GetId());
        }
    }

    m_tab_manager.HandleTabClose();
    m_input_state.Reset();

    ImGui::EndTabBar();
}

// @NOTE: do not hold the pointer
ViewerTab* Viewer::GetActiveTab() {
    auto active = m_tab_manager.GetActiveTab();

    if (active.is_none()) {
        return nullptr;
    }

    return active.unwrap();
}

}  // namespace cave
