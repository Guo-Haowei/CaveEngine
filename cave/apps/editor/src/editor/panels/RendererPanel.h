#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

class RendererPanel : public EditorWindow {
public:
    RendererPanel(EditorState& p_editor)
        : EditorWindow(p_editor) {}

    const char* GetWindowId() const override {
        return "Renderer";
    }

protected:
    void DrawUIImpl(float p_timestep) override;
};

}  // namespace cave
