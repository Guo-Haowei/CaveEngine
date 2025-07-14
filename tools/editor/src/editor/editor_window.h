#pragma once
#include "editor/editor_item.h"

namespace my {

inline constexpr const char* ASSET_DRAG_DROP_PAYLOAD = "ASSET_DRAG_DROP_PAYLOAD";

class EditorWindow : public EditorItem {
public:
    EditorWindow(const std::string& p_name, EditorLayer& p_editor)
        : EditorItem(p_editor), m_name(p_name) {}

    void Update(Scene*) override;

protected:
    virtual void UpdateInternal(Scene*) = 0;

    std::string m_name;
    int m_flags{ 0 };
};

}  // namespace my
