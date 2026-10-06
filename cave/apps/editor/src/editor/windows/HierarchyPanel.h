#pragma once
#include "cave/core/ids/Entity.h"

#include "editor/document/DocId.h"
#include "editor/windows/EditorWindow.h"

namespace cave {

class Scene;
struct SceneEditContext;

class HierarchyPanel : public EditorWindow {
public:
    HierarchyPanel(EditorState& editor)
        : EditorWindow(editor) {}

    const char* windowId() const override { return "Hierarchy"; }

private:
    void drawUIImpl() override;
    void drawToolbar(const SceneEditContext* context,
                     const Scene* scene);

    void drawPopup(const SceneEditContext& context,
                   const Scene& scene);

    void openAddEntityPopupImpl(const SceneEditContext& context,
                                const Scene& scene,
                                ecs::Entity parent);

    void openAddUIPopupImpl(const SceneEditContext& context,
                            const Scene& scene,
                            ecs::Entity parent);
};

}  // namespace cave
