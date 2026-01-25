#pragma once
#include "editor/EditorWindow.h"

namespace cave {

class RendererPanel : public EditorWindow {
public:
    RendererPanel(EditorState& p_editor)
        : EditorWindow(p_editor) {}

    const char* GetTitle() const override {
        return "Renderer";
    }

protected:
    void UpdateInternal(float p_timestep) override;
};

}  // namespace cave
