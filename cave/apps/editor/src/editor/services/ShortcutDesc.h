#pragma once
namespace cave {

enum class Key : uint16_t;

enum class Shortcut : uint8_t {
    SaveAs,  // Ctrl + Shift + S
    Save,    // Ctrl + S
    Open,    // Ctrl + O
    Redo,    // Ctrl + Shift + Z
    Undo,    // Ctrl + Z
    Debug,   // F5

    _Count,
};

constexpr int kShortcutCount = std::to_underlying(Shortcut::_Count);

struct ShortcutDesc {
    const char* name{ nullptr };
    const char* shortcut{ nullptr };
    std::function<void()> execute_func{ nullptr };
    std::function<bool()> enabled_func{ nullptr };

    Key key{};
    bool ctrl{};
    bool alt{};
    bool shift{};
};

}  // namespace cave
