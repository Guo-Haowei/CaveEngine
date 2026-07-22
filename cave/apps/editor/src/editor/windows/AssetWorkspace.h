#pragma once
#include "editor/windows/EditorWindow.h"

namespace cave {

struct EditorServices;

class LogPanel;
class TileSetPanel;

class AssetWorkspace final : public EditorWindow {
public:
    explicit AssetWorkspace(EditorState& editor);
    ~AssetWorkspace() override;

private:
    const char* windowId() const override {
        return "Asset Workspace";
    }

    void onAttach() override;

    void drawUIImpl() override;

    Owner<LogPanel> m_log;
    Owner<TileSetPanel> m_tile_set;
};

}  // namespace cave
