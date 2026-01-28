#pragma once
#include "engine/private/assets/guid.h"
#include "editor/Enums.h"
#include "editor/undo_redo/UndoStack.h"

namespace cave {

class CameraComponent;
class EditorState;
class Scene;
class Viewer;

class ViewerTabId {
public:
    constexpr ViewerTabId(int p_val)
        : m_val(p_val) {}

    static constexpr ViewerTabId Null() {
        return ViewerTabId(-1);
    }

    ViewerTabId(const ViewerTabId& p_rhs) = default;

    bool operator==(const ViewerTabId& p_rhs) const { return m_val == p_rhs.m_val; }

    int Get() const { return m_val; }

    static ViewerTabId Next() {
        static int s_counter = 0;
        return { ++s_counter };
    }

private:
    int m_val;
};

}  // namespace cave

namespace std {

template<>
struct hash<::cave::ViewerTabId> {
    std::size_t operator()(const ::cave::ViewerTabId& p_tab_id) const noexcept {
        return std::hash<int>{}(p_tab_id.Get());
    }
};

}  // namespace std
