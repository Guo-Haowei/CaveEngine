#pragma once
#include "editor/EditorItem.h"
#include "engine/private/scene/scene.h"

namespace cave {

class MenuBar : public EditorItem {
public:
    MenuBar(EditorState& p_editor)
        : EditorItem(p_editor) {}

    void Update(float p_timestep) override;
};

}  // namespace cave
