#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

class PropertyPanel : public EditorWindow {
public:
    PropertyPanel(EditorState& editor)
        : EditorWindow(editor) {}

    const char* windowId() const override {
        return "Properties";
    }

protected:
    void drawUIImpl() override;
};

}  // namespace cave
