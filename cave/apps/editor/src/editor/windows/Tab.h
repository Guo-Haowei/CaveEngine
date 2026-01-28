#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

class Tab : public EditorWindow {
public:
    Tab(EditorState& p_editor);

    const char* GetWindowId() const override { return m_window_id.c_str(); }

    void SetTitleAndId(std::string_view p_title, uint32_t p_idx);

protected:
    void UpdateInternal(float p_dt) override;

    std::string m_window_id;
    std::string m_title;
    uint32_t m_idx{ 0 };
};

}  // namespace cave
