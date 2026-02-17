#pragma once
#include "cave/runtime/ecs/Entity.h"
#include "editor/document/DocId.h"
#include "editor/panels/EditorWindow.h"

namespace cave {

class ViewerTab;

struct PreviewScene;

class HierarchyPanel : public EditorWindow {
public:
    HierarchyPanel(EditorState& editor)
        : EditorWindow(editor) {}

    const char* GetWindowId() const override {
        return "Hierarchy";
    }

protected:
    void DrawUIImpl() override;

private:
    void DrawPopup(const PreviewScene& p_ctx);
    void OpenAddEntityPopupImpl(DocId p_doc_id, ecs::Entity p_parent);
};

}  // namespace cave
