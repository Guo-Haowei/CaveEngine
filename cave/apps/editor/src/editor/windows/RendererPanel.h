#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

class RendererPanel : public EditorWindow {
public:
    RendererPanel(EditorState& p_editor)
        : EditorWindow(p_editor) {}

    const char* windowId() const override {
        return "Renderer";
    }

protected:
    void drawUIImpl() override;
};

}  // namespace cave
