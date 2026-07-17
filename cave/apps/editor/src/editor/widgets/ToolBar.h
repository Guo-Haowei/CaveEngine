#pragma once

namespace cave {

struct ToolBarButtonDesc {
    const char* id{ nullptr };
    const char* display{ nullptr };
    const char* tooltip{ nullptr };
    std::function<void()> execute_func;
    std::function<bool()> is_enabled_func;
    std::function<bool()> is_selected_func;
};

void DrawToolBar(std::span<const ToolBarButtonDesc*> button_descs,
                 bool new_line = false);

void DrawToolBar(std::span<const ToolBarButtonDesc> button_descs,
                 bool new_line = false);

}  // namespace cave
