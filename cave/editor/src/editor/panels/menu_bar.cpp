#include "menu_bar.h"

#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/input_manager.h"
#include "engine/ui/layout.h"

#include "editor/editor_state.h"
#include "editor/panels/log_panel.h"
#include "editor/widgets/image.h"

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
            CompositeLogger& logger = CompositeLogger::GetSingleton();
            const uint32_t error_count = static_cast<uint32_t>(logger.GetErrorLogs().size());
            const uint32_t warning_count = static_cast<uint32_t>(logger.GetWarningLogs().size());

            ui::ErrorIcon();

            ImGui::SameLine();
            ImGui::Text(" %u Error(s)", error_count);

            ImGui::SameLine();
            ui::WarningIcon();

            ImGui::SameLine();
            ImGui::Text(" %u Warning(s)", warning_count);

            ImGui::EndMenuBar();
        }
        ImGui::End();
    }
}

}  // namespace cave
