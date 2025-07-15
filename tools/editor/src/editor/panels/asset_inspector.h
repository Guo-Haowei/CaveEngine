#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "editor/editor_window.h"

namespace my {

class AssetRegistry;
struct IAsset;
struct ImageAsset;
class TileMapAsset;

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorLayer& p_editor);

    void OnAttach() override;

protected:
    void UpdateInternal(Scene*) override;

    void TileSetup(SpriteAsset& p_sprite);
    void TilePaint(SpriteAsset& p_sprite);

    // SpriteSheet
    void InspectSprite(IAsset* p_asset);
    void DropRegion(SpriteAsset& p_sprite);

    // TileMap
    void InspectTileMap(IAsset* p_asset);
    void TileMapLayerOverview(TileMapAsset& p_tile_map);

    // @TODO: refactor

    int m_selected_x = -1;
    int m_selected_y = -1;
    // @TODO: refactor

    AssetRegistry* m_asset_registry;

    Handle<ImageAsset> m_checkerboard_handle;
};

}  // namespace my
