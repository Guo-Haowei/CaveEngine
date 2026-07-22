#pragma once
#include "cave/core/ids/Entity.h"

#include "editor/document/DocId.h"
#include "editor/windows/EditorWindow.h"

namespace cave {

class ViewerTab;

struct PreviewScene;

class HierarchyPanel : public EditorWindow {
public:
    HierarchyPanel(EditorState& editor)
        : EditorWindow(editor) {}

    const char* windowId() const override { return "Hierarchy"; }

private:
    void drawUIImpl() override;
    void drawToolbar();

    void drawPopup(const PreviewScene& preview_scene);
    void openAddEntityPopupImpl(const PreviewScene& preview_scene, ecs::Entity parent);
    void openAddUIPopupImpl(const PreviewScene& preview_scene, ecs::Entity parent);
};

}  // namespace cave
