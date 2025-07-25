#pragma once
#include "editor/editor_window.h"

namespace cave {

class ViewerTab;

class HierarchyPanel : public EditorWindow {
public:
    HierarchyPanel(EditorLayer& editor)
        : EditorWindow(editor) {}

    const char* GetTitle() const override {
        return "Hierarchy";
    }

protected:
    void UpdateInternal() override;

private:
    void DrawPopup(ViewerTab* p_tab);
};

}  // namespace cave
