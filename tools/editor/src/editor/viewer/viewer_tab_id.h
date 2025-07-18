#pragma once
#include "engine/assets/guid.h"
#include "editor/enums.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

class CameraComponent;
class EditorLayer;
class InputEvent;
class Scene;
class Viewer;

class TabId {
public:
    constexpr TabId(int p_val)
        : m_val(p_val) {}

    static constexpr TabId Null() {
        return TabId(-1);
    }

    TabId(const TabId& p_rhs) = default;

    bool operator==(const TabId& p_rhs) const { return m_val == p_rhs.m_val; }

    int Get() const { return m_val; }

    static TabId Next() {
        static int s_counter = 0;
        return { ++s_counter };
    }

private:
    int m_val;
};

}  // namespace cave

namespace std {

template<>
struct hash<::cave::TabId> {
    std::size_t operator()(const ::cave::TabId& p_tab_id) const noexcept {
        return std::hash<int>{}(p_tab_id.Get());
    }
};

}  // namespace std
