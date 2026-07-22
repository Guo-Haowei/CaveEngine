#pragma once
#include <filesystem>

#include "cave/core/time/CountdownTimer.h"

#include "editor/windows/Tab.h"

namespace cave {

struct ContentBrowserState {
    std::string current_path = "@res://";
};

class WorkspaceState {
public:
    ContentBrowserState content_browser;

    std::vector<TabState> tabs;

    bool load(const std::filesystem::path& path);
    bool save(const std::filesystem::path& path, float dt);
    bool saveNow(const std::filesystem::path& path);

    void markDirty();

private:
    bool saveImpl(const std::filesystem::path& path) const;

    bool m_dirty = false;
    CountdownTimer m_timer{ 10 };
};

}  // namespace cave