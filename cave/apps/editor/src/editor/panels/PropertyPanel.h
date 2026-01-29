#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

class PropertyPanel : public EditorWindow {
public:
    PropertyPanel(EditorState& p_editor)
        : EditorWindow(p_editor) {}

    const char* GetWindowId() const override {
        return "Properties";
    }

protected:
    void DrawUIImpl() override;
};

}  // namespace cave
