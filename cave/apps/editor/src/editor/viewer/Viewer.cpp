#include "Viewer.h"

#include <imgui/imgui_internal.h>

#include "engine/private/debugger/profiler.h"
#include "engine/private/math/ray.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "engine/private/runtime/framework/ViewportManager.h"

#include "editor/document/document.h"
#include "editor/EditorDvars.h"
#include "editor/EditorState.h"
#include "editor/utility/ImGuizmo.h"
#include "engine/private/ui/layout.h"

// asset editors
#include "editor/animation_editor/SpriteAnimationEditor.h"
#include "editor/material_editor/MaterialEditor.h"
#include "editor/scene_editor/SceneEditor.h"
#include "editor/tile_map_editor/TileMapEditor.h"
#include "editor/tile_map_editor/TileSetEditor.h"

namespace cave {

#define VIEWER_WINDOW_ID "###Viewer"

static constexpr float TOOL_BAR_OFFSET = 80.0f;

Viewer::Viewer(EditorState& p_editor)
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

Option<Vector2f> Viewer::CursorToNDC(Vector2f p_point) const {
    auto [window_x, window_y] = m_editor.GetApp().GetDisplayManager()->GetWindowPos();
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
        case AssetType::Scene: {
            ViewerTab::Dimension dimension = DVAR_GET_BOOL(is_world_2d) ? ViewerTab::DIMENSION_2
                                                                        : ViewerTab::DIMENSION_3;
            tab.reset(new SceneEditor(m_editor, *this, dimension));
        } break;
        case AssetType::TileSet: {
            tab.reset(new TileSetEditor(m_editor, *this));
        } break;
        case AssetType::TileMap: {
            tab.reset(new TileMapEditor(m_editor, *this));
        } break;
        case AssetType::SpriteAnimation: {
            tab.reset(new SpriteAnimationEditor(m_editor, *this));
        } break;
        case AssetType::Material: {
            tab.reset(new MaterialEditor(m_editor, *this));
        } break;
        default:
            LOG_WARN("Can't open tab {}", EnumTraits<AssetType>::ToString(p_type));
            return;
    }

    ViewportManager* viewport_manager = m_editor.GetApp().GetViewportManager();
    viewport_manager->CreateViewport(tab);

    DVAR_SET_STRING(last_open_asset, p_guid.ToString());

    tab->OnCreate(p_guid);
    m_tab_manager.SwitchTab(std::move(tab));
}

void Viewer::UpdateInternal(float p_timestep) {
    CAVE_PROFILE_EVENT();

    UpdateFrameSize();

    auto _tab = m_tab_manager.GetActiveTab();
    if (_tab.is_none()) {
        return;
    }

    ViewerTab* active_tab = _tab.unwrap_unchecked();
    active_tab->Update(p_timestep);

    int flag = 0;
#if 0
    flag |= ImGuiTabBarFlags_Reorderable;
#endif
    if (!ImGui::BeginTabBar("MyTabs", flag)) {
        return;
    }

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

            auto buttons = tab->GetToolBarButtons();
            DrawToolBar(buttons);

            // @TODO: remove this dummy camera
            CameraComponent dummy_camera;
            tab->DrawMainView(dummy_camera);

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
