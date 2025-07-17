#pragma once
#include "editor/editor_item.h"

namespace cave {

inline constexpr const char* ASSET_DRAG_DROP_PAYLOAD = "ASSET_DRAG_DROP_PAYLOAD";

class EditorWindow : public EditorItem {
public:
    EditorWindow(EditorLayer& p_editor)
        : EditorItem(p_editor) {}

    void Update(Scene*) override;

    virtual const char* GetTitle() const = 0;

protected:
    virtual void UpdateInternal(Scene*) = 0;

    int m_flags{ 0 };
};

}  // namespace cave
