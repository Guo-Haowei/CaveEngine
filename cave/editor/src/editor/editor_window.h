#pragma once
#include "editor/editor_item.h"

namespace cave {

class EditorWindow : public EditorItem {
public:
    EditorWindow(EditorLayer& p_editor)
        : EditorItem(p_editor) {}

    void Update() override;

    virtual const char* GetTitle() const = 0;

    bool IsFocused() const { return m_is_focused; }
    bool IsHovered() const { return m_is_hovered; }

protected:
    virtual void UpdateInternal() = 0;

    bool m_is_focused = false;
    bool m_is_hovered = false;

    int m_flags{ 0 };
};

}  // namespace cave
