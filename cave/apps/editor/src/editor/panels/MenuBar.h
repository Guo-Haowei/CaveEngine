#pragma once
#include "editor/IEditorItem.h"

namespace cave {

class MenuBar : public IEditorItem {
public:
    MenuBar(EditorState& p_editor)
        : IEditorItem(p_editor) {}

    void DrawUI() override;
};

}  // namespace cave
