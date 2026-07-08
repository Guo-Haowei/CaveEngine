#pragma once
#include "cave/core/ids/Entity.h"
#include "editor/document/DocId.h"
#include "editor/panels/EditorWindow.h"

namespace cave {

class ViewerTab;

struct PreviewScene;

class HierarchyPanel : public EditorWindow {
public:
    HierarchyPanel(EditorState& editor)
        : EditorWindow(editor) {}

    const char* windowId() const override { return "Hierarchy"; }

protected:
    void drawUIImpl() override;

private:
    void drawPopup(const PreviewScene& ctx);
    void openAddEntityPopupImpl(DocId doc_id, ecs::Entity parent);
};

}  // namespace cave
