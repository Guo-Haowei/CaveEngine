#pragma once
#include "editor/IEditorItem.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class MenuBar : public IEditorItem {
public:
    MenuBar(EditorState& p_editor)
        : IEditorItem(p_editor) {}

    void Update(float p_timestep) override;
};

}  // namespace cave
