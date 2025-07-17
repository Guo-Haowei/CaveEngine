#pragma once
#include "editor/editor_window.h"

namespace cave {

class RendererPanel : public EditorWindow {
public:
    RendererPanel(EditorLayer& p_editor)
        : EditorWindow(p_editor) {}

    const char* GetTitle() const override {
        return "Renderer";
    }

protected:
    void UpdateInternal(Scene* p_scene) override;
};

}  // namespace cave
