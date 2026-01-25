#pragma once
#include "editor/editor_item.h"
#include "engine/scene/scene.h"

namespace cave {

class MenuBar : public EditorItem {
public:
    MenuBar(EditorState& p_editor)
        : EditorItem(p_editor) {}

    void Update(float p_timestep) override;
};

}  // namespace cave
