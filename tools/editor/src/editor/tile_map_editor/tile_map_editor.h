#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/geomath.h"
#include "engine/tile_map/tile_map_asset.h"

#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/sprite_selector.h"

namespace cave {

class AssetRegistry;
class CameraComponent;
class Document;
class InputEvent;
class Scene;
class TileMapAsset;
class TileMapDocument;
class Viewer;

class TileMapEditor : public ViewerTab {
public:
    TileMapEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) override;

    void OnCreate(const Guid& p_guid) override;

    void OnDestroy() override;

    void OnActivate() override;

    void DrawMainView(const CameraComponent& p_camera) override;

    void DrawAssetInspector() override;

    Document& GetDocument() const override;

    bool CursorToTile(const Vector2f& p_in, TileIndex& p_out) const;

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    const std::vector<ToolBarButtonDesc>& GetToolBarButtons() const override;

    void UndoableSetTile(TileMapAsset& p_layer,
                         int p_layer_id,
                         TileIndex p_index,
                         Option<TileId> p_new_tile);

    // @TODO: refactor
    void TileMapLayerOverview(TileMapAsset& p_tile_map);

    Handle<ImageAsset> m_checkerboard_handle;

    AssetRegistry* m_asset_registry;

    std::shared_ptr<Scene> m_tmp_scene;

    std::unique_ptr<CameraComponent> m_camera;
    std::shared_ptr<TileMapDocument> m_document;

    SpriteSelector m_sprite_selector;
};

}  // namespace cave
