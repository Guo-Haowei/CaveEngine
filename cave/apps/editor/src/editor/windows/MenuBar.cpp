#include "MenuBar.h"

#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/ui/Layout.h"

#include "editor/EditorState.h"
#include "editor/services/ShortcutService.h"
#include "editor/windows/LogPanel.h"
#include "editor/widgets/Image.h"

namespace cave {

void MenuBar::drawUI() {
    const auto& shortcuts = m_editor_services.shortcut().getShortcuts();
    auto build_menu_item = [&](Shortcut p_index) {
        const auto& it = shortcuts[std ::to_underlying(p_index)];
        const bool enabled = it.enabled_func ? it.enabled_func() : true;
        if (ImGui::MenuItem(it.name, it.shortcut, false, enabled)) {
            it.execute_func();
        }
    };

    if (ImGui::BeginMenu("File")) {
        build_menu_item(Shortcut::Open);
        // Open Recent
        ImGui::Separator();
        build_menu_item(Shortcut::Save);
        build_menu_item(Shortcut::SaveAs);
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Edit")) {
        build_menu_item(Shortcut::Undo);
        build_menu_item(Shortcut::Redo);
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
    if (ImGui::BeginMenu("Debug")) {
        build_menu_item(Shortcut::Debug);
        ImGui::EndMenu();
    }
}

}  // namespace cave
