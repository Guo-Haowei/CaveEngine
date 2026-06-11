#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

class PropertyPanel : public EditorWindow {
public:
    PropertyPanel(EditorState& p_editor)
        : EditorWindow(p_editor) {}

    const char* windowId() const override {
        return "Properties";
    }

protected:
    void drawUIImpl() override;
};

}  // namespace cave
