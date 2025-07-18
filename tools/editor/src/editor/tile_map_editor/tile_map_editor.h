#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/input/input_event.h"
#include "engine/scene/scene.h"
#include "engine/tile_map/tile_map_asset.h"

// @TODO: move to tile_map_editor folder?
#include "editor/viewer/viewer_tab.h"

namespace cave {

class AssetRegistry;
class Document;
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

    void Draw() override;

    Document& GetDocument() const override;

    bool CursorToTile(const Vector2f& p_in, TileIndex& p_out) const;

protected:
    void UndoableSetTile(TileMapAsset& p_layer,
                         int p_layer_id,
                         TileIndex p_index,
                         Option<TileId> p_new_tile);

    const CameraComponent& GetActiveCameraInternal() const override;

    AssetRegistry* m_asset_registry;

    std::shared_ptr<Scene> m_tmp_scene;

    std::shared_ptr<CameraComponent> m_camera;

    std::shared_ptr<TileMapDocument> m_document;
};

}  // namespace cave
