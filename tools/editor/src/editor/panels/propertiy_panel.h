#pragma once
#include "editor/editor_window.h"

namespace cave {

class PropertyPanel : public EditorWindow {
public:
    PropertyPanel(EditorLayer& p_editor)
        : EditorWindow("Properties", p_editor) {}

protected:
    void UpdateInternal(Scene* p_scene) override;
};

}  // namespace cave
