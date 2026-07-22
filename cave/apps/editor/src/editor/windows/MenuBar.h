#pragma once
#include "editor/IEditorItem.h"

namespace cave {

class MenuBar : public IEditorItem {
public:
    MenuBar(EditorState& editor)
        : IEditorItem(editor) {}

    void drawUI() override;
};

}  // namespace cave
