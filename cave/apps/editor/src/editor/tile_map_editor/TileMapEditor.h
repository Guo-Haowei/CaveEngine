#pragma once
#include "engine/private/assets/asset_handle.h"
#include "engine/private/math/geomath.h"
#include "engine/private/assets/tile_map_asset.h"

#include "editor/viewer/ViewerTab.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

#if 0
class AssetRegistry;
class Scene;
class TileMapDocument;

class TileMapEditor : public ViewerTab {
public:
    TileMapEditor(EditorState& p_editor, Viewer& p_viewer);
    ~TileMapEditor();

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    void DrawAssetInspector() final;

    bool CursorToTile(const Vector2f& p_in, TileIndex& p_out) const;

protected:
    void OnCreateInternal(const Guid& p_guid) final;

    void OnActivateInternal() final;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const override;

    // @TODO: refactor
    void TileMapLayerOverview(TileMapAsset& p_tile_map);

    std::shared_ptr<Scene> m_tmp_scene;

    std::unique_ptr<CameraComponent> m_camera;
    std::unique_ptr<TileMapDocument> m_document;
    ToolBarButtonDesc m_brush_desc;

    SpriteSelector m_sprite_selector;
};
#endif

}  // namespace cave
