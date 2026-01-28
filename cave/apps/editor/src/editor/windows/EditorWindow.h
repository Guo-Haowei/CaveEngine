#pragma once
#include "editor/IEditorItem.h"

namespace cave {

struct WindowState {
    bool open{ false };
    bool visible{ false };
    bool hovered{ false };
    bool focused{ false };
};

template<typename T>
struct RectT {
    T x{};
    T y{};
    T w{};
    T h{};

    T Left() const { return x; }
    T Right() const { return x + w; }

    T Top() const { return y; }
    T Bottom() const { return y + h; }
};

using Rect32f = RectT<float>;

class EditorWindow : public IEditorItem {
public:
    EditorWindow(EditorState& p_editor)
        : IEditorItem(p_editor) {}

    void DrawUI(float p_timestep) override;

    virtual const char* GetWindowId() const = 0;

    bool IsOpen() const { return m_state.open; }
    bool IsVisible() const { return m_state.visible; }
    bool IsFocused() const { return m_state.focused; }
    bool IsHovered() const { return m_state.hovered; }

protected:
    virtual void DrawUIImpl(float p_timestep) = 0;

    WindowState m_state;

    Rect32f m_rect{};

    int m_flags{ 0 };
};

}  // namespace cave
