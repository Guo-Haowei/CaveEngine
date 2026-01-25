#pragma once
#include "editor/editor_item.h"

namespace cave {

class EditorWindow : public EditorItem {
public:
    EditorWindow(EditorState& p_editor)
        : EditorItem(p_editor) {}

    void Update(float p_timestep) override;

    virtual const char* GetTitle() const = 0;

    bool IsFocused() const { return m_is_focused; }
    bool IsHovered() const { return m_is_hovered; }

protected:
    virtual void UpdateInternal(float p_timestep) = 0;

    bool m_is_focused = false;
    bool m_is_hovered = false;

    int m_flags{ 0 };
};

}  // namespace cave
