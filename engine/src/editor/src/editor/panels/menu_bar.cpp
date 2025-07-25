#include "menu_bar.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/input_manager.h"

#include "editor/editor_layer.h"
#include "editor/panels/log_panel.h"
#include "editor/widgets/widget.h"

namespace cave {

void MenuBar::MainMenuBar() {
    const auto& shortcuts = m_editor.GetShortcuts();
    auto build_menu_item = [&](int p_index) {
        const auto& it = shortcuts[p_index];
        const bool enabled = it.enabledFunc ? it.enabledFunc() : true;
        if (ImGui::MenuItem(it.name, it.shortcut, false, enabled)) {
            it.executeFunc();
        }
    };

    if (ImGui::BeginMenu("File")) {
        build_menu_item(SHORT_CUT_OPEN);
        // Open Recent
        ImGui::Separator();
        build_menu_item(SHORT_CUT_SAVE);
        build_menu_item(SHORT_CUT_SAVE_AS);
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Edit")) {
        build_menu_item(SHORT_CUT_UNDO);
        build_menu_item(SHORT_CUT_REDO);
        ImGui::Separator();
        if (ImGui::MenuItem("Cut", "Ctrl+X")) {
        }
        if (ImGui::MenuItem("Copy", "Ctrl+C")) {
        }
        if (ImGui::MenuItem("Paste", "Ctrl+V")) {
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();
    EditorItem::OpenAddEntityPopup(ecs::Entity::Null());
}

void MenuBar::Update() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetCurrentViewport(nullptr, (ImGuiViewportP*)viewport);  // Set viewport explicitly so GetFrameHeight reacts to DPI changes

    float height = ImGui::GetFrameHeight();

    if (ImGui::BeginMainMenuBar()) {
        MainMenuBar();
        ImGui::EndMainMenuBar();
    }

    if (ImGui::BeginViewportSideBar("StatusBar", viewport, ImGuiDir_Down, height, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar)) {
        if (ImGui::BeginMenuBar()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.2f, 0.2f, 1.0f));
            ImGui::Text(ICON_FA_CIRCLE_EXCLAMATION);
            ImGui::PopStyleColor();

            ImGui::SameLine();

            const uint32_t error_count = m_editor.GetLogPanel().GetErrorCount();
            const uint32_t warning_count = m_editor.GetLogPanel().GetWarningCount();

            ImGui::Text(" %u Error", error_count);

            ImGui::SameLine();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.75f, 0.0f, 1.0f));
            ImGui::Text(ICON_FA_TRIANGLE_EXCLAMATION);
            ImGui::PopStyleColor();

            ImGui::SameLine();

            ImGui::Text(" %u Warning", warning_count);

            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
}

}  // namespace cave
