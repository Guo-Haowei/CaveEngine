#pragma once
#include "editor/panels/EditorWindow.h"
#include "editor/tile_map_editor/TileMapEditorContext.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

struct EditorServices;

class IDocument;
class SpriteAnimationAsset;
class TileMapAsset;
class TileMapDocument;
class TileSetAsset;

struct SpriteAnimationContext {
    FixedString<64> clip_name;
    SpriteSelector sprite_selector{ SpriteSelector::SelectionMode::Multi };
};

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
    void drawSpriteAnimation(IDocument& doc);

    void tileMapLayerOverview(TileMapAsset& tile_map);

    EditorServices& editor_services_;

    uint64_t checkerboard_{};

    TileMapEditorContext tile_map_ctx_;
    SpriteAnimationContext sprite_animation_ctx_;
};

}  // namespace cave
