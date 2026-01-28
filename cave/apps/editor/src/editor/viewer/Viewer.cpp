#include "Viewer.h"

#include <imgui/imgui_internal.h>

#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"

// -----------------------------
#include "engine/private/debugger/profiler.h"
#include "engine/private/math/ray.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/runtime/framework/DisplayManager.h"
#include "engine/private/runtime/framework/ViewportManager.h"

#include "editor/EditorDvars.h"
#include "editor/EditorState.h"
#include "editor/utility/ImGuizmo.h"
#include "engine/private/ui/layout.h"

// asset editors
#include "editor/animation_editor/SpriteAnimationEditor.h"
#include "editor/material_editor/MaterialEditor.h"
#include "editor/tile_map_editor/TileMapEditor.h"
#include "editor/tile_map_editor/TileSetEditor.h"

namespace cave {

#if 0
#define VIEWER_WINDOW_ID "###Viewer"

static constexpr float TOOL_BAR_OFFSET = 80.0f;

Viewer::Viewer(EditorState& p_editor)
    : EditorWindow(p_editor)
    , m_workspace(p_editor.Workspace()) {
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

    ImGuiWindow* window = ImGui::FindWindowByName(GetWindowId());
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

ViewerTab* Viewer::GetActiveTab() {
    return nullptr;
}

void Viewer::UpdateInternal(float p_timestep) {
    CAVE_PROFILE_EVENT();

    UpdateFrameSize();

    ViewerTab* active_tab = nullptr;
    if (!active_tab) {
        return;
    }

    active_tab->Update(p_timestep);

    int flag = 0;
#if 0
    flag |= ImGuiTabBarFlags_Reorderable;
#endif
    if (!ImGui::BeginTabBar("MyTabs", flag)) {
        return;
    }

// TabId focus_tab_id = m_workspace.GetFocusRequest().unwrap_or(TabId::Null());
#if 0
    EditService& edit = m_editor.EditService();
    for (auto& [id, tab] : m_workspace.GetTabs()) {
        DocId doc = tab->GetDocId();
        int flags = 0;
        flags |= edit.IsDirty(doc) ? ImGuiTabItemFlags_UnsavedDocument : 0;

        // if (tab->GetId() == focus_tab_id) {
        //     flags |= ImGuiTabItemFlags_SetSelected;
        //     m_workspace.ClearFocusRequest();
        // }

        bool tab_open = true;
        if (ImGui::BeginTabItem(tab->GetTitle().c_str(), &tab_open, flags)) {
            // if (focus_tab_id == TabId::Null()) {
            //     LOG_WARN("TODO");
            //     //m_workspace.SwitchTab(tab->GetId());
            // }

            auto buttons = tab->GetToolBarButtons();
            DrawToolBar(buttons);

            // @TODO: remove this dummy camera
            CameraComponent dummy_camera;
            tab->DrawMainView(dummy_camera);

            ImGui::EndTabItem();
        }

        if (!tab_open) {
            // m_workspace.SetCloseRequest(tab->GetId());
        }
    }
#endif

    m_workspace.HandleCloseRequest();

    ImGui::EndTabBar();
}
#endif

}  // namespace cave
