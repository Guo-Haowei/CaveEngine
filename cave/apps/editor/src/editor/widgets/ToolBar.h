#pragma once

namespace cave {

struct ToolbarButtonDesc {
    const char* id{ nullptr };
    const char* display{ nullptr };
    const char* tooltip{ nullptr };
    std::function<void()> execute_func;
    std::function<bool()> is_enabled_func;
    std::function<bool()> is_selected_func;
};

void DrawToolbar(std::span<const ToolbarButtonDesc*> button_descs);

void DrawToolbar(std::span<const ToolbarButtonDesc> button_descs);

}  // namespace cave
