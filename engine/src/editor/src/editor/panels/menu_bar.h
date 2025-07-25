#pragma once
#include "editor/editor_item.h"
#include "engine/scene/scene.h"

namespace cave {

class MenuBar : public EditorItem {
public:
    MenuBar(EditorLayer& p_editor)
        : EditorItem(p_editor) {}

    void Update() override;

private:
    void MainMenuBar();
};

}  // namespace cave
