#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

struct EditorServices;

class TileMapAsset;
class TileMapDocument;

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorState& editor,
                   EditorServices& editor_services);

    const char* windowId() const override {
        return "Asset Inspector";
    }

    void onAttach() override;

protected:
    void drawUIImpl() override;

    void drawDocument(TileMapDocument& doc);

    void tileMapLayerOverview(TileMapAsset& tile_map);

    EditorServices& editor_services_;

    uint64_t checkerboard_{};
};

}  // namespace cave
