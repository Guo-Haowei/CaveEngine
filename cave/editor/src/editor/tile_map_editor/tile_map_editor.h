#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/math/geomath.h"
#include "engine/assets/tile_map_asset.h"

#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/sprite_selector.h"

namespace cave {

class AssetRegistry;
class Document;
class InputEvent;
class Scene;
class TileMapDocument;

class TileMapEditor : public ViewerTab {
public:
    TileMapEditor(EditorLayer& p_editor, Viewer& p_viewer);
    ~TileMapEditor();

    bool HandleInput(const InputEvent* p_input_event) final;

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    void DrawAssetInspector() final;

    Document& GetDocument() const final;

    bool CursorToTile(const Vector2f& p_in, TileIndex& p_out) const;

    Scene* GetScene() final {
        return m_tmp_scene.get();
    }

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

}  // namespace cave
