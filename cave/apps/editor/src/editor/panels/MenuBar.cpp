#include "MenuBar.h"

#include "cave/runtime/framework/IInputService.h"

#include "engine/private/render/render_device/RenderDevice.h"
#include "engine/private/ui/layout.h"

#include "editor/EditorState.h"
#include "editor/services/ShortcutService.h"
#include "editor/panels/LogPanel.h"
#include "editor/widgets/Image.h"

namespace cave {

void MenuBar::DrawUI() {
    const auto& shortcuts = m_editor.ShortcutService().GetShortcuts();
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

    m_editor.OpenAddEntityPopup(ecs::Entity::Null());

    ImGui::Separator();
    if (ImGui::BeginMenu("Debug")) {
        build_menu_item(Shortcut::Debug);
        ImGui::EndMenu();
    }
}

}  // namespace cave
