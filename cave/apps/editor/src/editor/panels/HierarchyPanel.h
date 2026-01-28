#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

class ViewerTab;

struct FocusedPreviewScene;

class HierarchyPanel : public EditorWindow {
public:
    HierarchyPanel(EditorState& editor)
        : EditorWindow(editor) {}

    const char* GetWindowId() const override {
        return "Hierarchy";
    }

protected:
    void DrawUIImpl(float p_timestep) override;

private:
    void DrawPopup(const FocusedPreviewScene& p_ctx);
};

}  // namespace cave
