#pragma once
#include "editor/editor_window.h"

namespace cave {

class HierarchyPanel : public EditorWindow {
public:
    HierarchyPanel(EditorLayer& editor)
        : EditorWindow("Scene", editor) {}

protected:
    void UpdateInternal(Scene* scene) override;

private:
    void DrawPopup(Scene& scene);
};

}  // namespace cave
