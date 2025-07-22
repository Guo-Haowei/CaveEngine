#include "viewer.h"

#include <imgui/imgui_internal.h>

#include "engine/input/input_event.h"
#include "engine/math/ray.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/runtime/common_dvars.h"
#include "engine/runtime/display_manager.h"
#include "engine/runtime/mode_manager.h"

#include "editor/document/document.h"
#include "editor/editor_layer.h"
#include "editor/utility/imguizmo.h"
#include "editor/widgets/widget.h"

// asset editors
#include "editor/animation_editor/sprite_animation_editor.h"
#include "editor/tile_map_editor/tile_map_editor.h"
#include "editor/viewer/scene_editor.h"

namespace cave {

#define VIEWER_WINDOW_ID "###Viewer"

static constexpr float TOOL_BAR_OFFSET = 80.0f;

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
}

bool Viewer::HandleInput(const InputEvent* p_input_event) {
    const GameMode mode = m_editor.GetApplication()->GetModeManager().GetMode();
    if (mode != GameMode::Editor) {
        return true;
    }

    auto active_tab = GetActiveTab();
    if (active_tab && active_tab->HandleInput(p_input_event)) {
        return true;
    }

    return CacheCameraInput(p_input_event);
}

Option<Vector2f> Viewer::CursorToNDC(Vector2f p_point) const {
    auto [window_x, window_y] = m_editor.GetApplication()->GetDisplayServer()->GetWindowPos();
    p_point.x = (p_point.x + window_x - m_canvas_min.x) / m_canvas_size.x;
    p_point.y = (p_point.y + window_y - m_canvas_min.y) / m_canvas_size.y;

    if (p_point.x >= 0.0f && p_point.x <= 1.0f && p_point.y >= 0.0f && p_point.y <= 1.0f) {
        p_point *= 2.0f;
        p_point -= 1.0f;
        p_point.y = -p_point.y;
        return Some(p_point);
    }

    return None();
}

bool Viewer::CacheCameraInput(const InputEvent* p_input_event) {
    if (auto e = dynamic_cast<const InputEventKey*>(p_input_event); e) {
        if (e->IsHolding() && !e->IsModiferPressed()) {
            bool handled = true;
            switch (e->GetKey()) {
                case KeyCode::KEY_D:
                    ++m_camera_input.dx;
                    break;
                case KeyCode::KEY_A:
                    --m_camera_input.dx;
                    break;
                case KeyCode::KEY_E:
                    ++m_camera_input.dy;
                    break;
                case KeyCode::KEY_Q:
                    --m_camera_input.dy;
                    break;
                case KeyCode::KEY_W:
                    ++m_camera_input.dz;
                    break;
                case KeyCode::KEY_S:
                    --m_camera_input.dz;
                    break;
                default:
                    handled = false;
                    break;
            }
            return handled;
        }
    }

    if (auto e = dynamic_cast<const InputEventMouseWheel*>(p_input_event); e) {
        if (!e->IsModiferPressed()) {
            m_camera_input.scroll += 3.0f * e->GetWheelY();
            return true;
        }
    }

    if (auto e = dynamic_cast<const InputEventMouseMove*>(p_input_event); e) {
        if (!e->IsModiferPressed() && e->IsButtonDown(MouseButton::MIDDLE)) {
            m_camera_input.mouse_move += e->GetDelta();
            return true;
        }
    }

    return false;
}

void Viewer::OpenTab(AssetType p_type, const Guid& p_guid) {
    // check if tab already exists
    auto cached_tab = m_tab_manager.FindTabByGuid(p_guid);

    if (cached_tab.is_some()) {
        m_tab_manager.SwitchTab(cached_tab.unwrap_unchecked()->GetId());
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
        case AssetType::SpriteAnimation:
            tab.reset(new SpriteAnimationEditor(m_editor, *this));
            break;
        default:
            LOG_WARN("Can't open tab {}", ToString(p_type));
            return;
    }

    tab->OnCreate(p_guid);
    m_tab_manager.SwitchTab(std::move(tab));
}

void Viewer::UpdateInternal(Scene*) {
    UpdateFrameSize();

    auto _tab = m_tab_manager.GetActiveTab();
    if (_tab.is_none()) {
        return;
    }

    ViewerTab* active_tab = _tab.unwrap_unchecked();

    int flag = 0;
#if 0
    flag |= ImGuiTabBarFlags_Reorderable;
#endif
    if (!ImGui::BeginTabBar("MyTabs", flag)) {
        return;
    }

    // update camera
    const float dt = m_editor.context.timestep;
    auto& camera = active_tab->GetActiveCamera();
    const auto& c = m_camera_input;
    const bool is_2d = camera.IsView2D();
    if (is_2d) {
        const float speed = dt * 0.5f;
        const float dx = speed * -c.mouse_move.x;
        const float dy = speed * c.mouse_move.y;
        CameraInputState state = {
            .move = Vector3f(dx, dy, 0.0f),
            .zoomDelta = -dt * c.scroll,
            .rotation = Vector2f::Zero,
        };
        m_controller_2d.Update(camera, state);
        camera.Update();
    } else {
        CameraInputState state{
            .move = dt * Vector3f(c.dx, c.dy, c.dz),
            .zoomDelta = dt * c.scroll,
            .rotation = dt * c.mouse_move,
        };
        m_controller_3d.Update(camera, state);
        camera.Update();
    }

    m_camera_input.Reset();

    TabId focus_tab_id = m_tab_manager.GetFocusRequest().unwrap_or(TabId::Null());
    for (auto& [id, tab] : m_tab_manager.GetTabs()) {
        int flags = 0;
        if (tab->GetDocument().IsDirty()) {
            flags |= ImGuiTabItemFlags_UnsavedDocument;
        }

        if (tab->GetId() == focus_tab_id) {
            flags |= ImGuiTabItemFlags_SetSelected;
            m_tab_manager.ClearFocusRequest();
        }

        bool tab_open = true;
        if (ImGui::BeginTabItem(tab->GetTitle().c_str(), &tab_open, flags)) {
            if (focus_tab_id == TabId::Null()) {
                m_tab_manager.SwitchTab(tab->GetId());
            }

            tab->DrawToolBar();
            tab->DrawMainView(camera);

            ImGui::EndTabItem();
        }

        if (!tab_open) {
            m_tab_manager.SetCloseRequest(tab->GetId());
        }
    }

    m_tab_manager.HandleCloseRequest();

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
