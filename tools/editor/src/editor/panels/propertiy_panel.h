#pragma once
#include "editor/editor_window.h"

namespace cave {

class PropertyPanel : public EditorWindow {
public:
    PropertyPanel(EditorLayer& p_editor)
        : EditorWindow(p_editor) {}

    const char* GetTitle() const override {
        return "Properties";
    }

protected:
    void UpdateInternal(Scene* p_scene) override;
};

}  // namespace cave
