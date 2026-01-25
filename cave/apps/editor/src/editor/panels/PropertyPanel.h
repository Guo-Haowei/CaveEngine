#pragma once
#include "editor/EditorWindow.h"

namespace cave {

class PropertyPanel : public EditorWindow {
public:
    PropertyPanel(EditorState& p_editor)
        : EditorWindow(p_editor) {}

    const char* GetTitle() const override {
        return "Properties";
    }

protected:
    void UpdateInternal(float p_timestep) override;
};

}  // namespace cave
