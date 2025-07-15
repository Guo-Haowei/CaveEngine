#pragma once
#include "editor/editor_window.h"

namespace cave {

class RendererPanel : public EditorWindow {
public:
    RendererPanel(EditorLayer& p_editor)
        : EditorWindow("Renderer", p_editor) {}

protected:
    void UpdateInternal(Scene* p_scene) override;
};

}  // namespace cave
