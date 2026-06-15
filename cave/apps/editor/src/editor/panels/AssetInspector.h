#pragma once
#include "editor/panels/EditorWindow.h"
#include "editor/tile_map_editor/TileMapEditorContext.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

struct EditorServices;

class TileMapAsset;
class TileSetAsset;
class IDocument;
class TileMapDocument;

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorState& editor,
                   EditorServices& editor_services);

    const char* windowId() const override {
        return "Asset Inspector";
    }

    void onAttach() override;

    TileMapEditorContext& tileMapContext() { return tile_map_ctx_; }

protected:
    void drawUIImpl() override;

    void drawTileMap(TileMapDocument& doc);
    void drawTileSet(IDocument& doc);

    void tileMapLayerOverview(TileMapAsset& tile_map);

    EditorServices& editor_services_;

    uint64_t checkerboard_{};

    TileMapEditorContext tile_map_ctx_;
};

}  // namespace cave
