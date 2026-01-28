#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

class PropertyPanel : public EditorWindow {
public:
    PropertyPanel(EditorState& p_editor)
        : EditorWindow(p_editor) {}

    const char* GetWindowId() const override {
        return "Properties";
    }

protected:
    void DrawUIImpl(float p_timestep) override;
};

}  // namespace cave
