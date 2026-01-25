#pragma once

namespace cave {

struct ToolBarButtonDesc {
    const char* display{ nullptr };
    const char* tooltip{ nullptr };
    std::function<void()> execute_func;
    std::function<bool()> is_enabled_func;
};

void DrawToolBar(const std::vector<const ToolBarButtonDesc*>& p_button_descs, bool p_new_line = false);

}  // namespace cave
