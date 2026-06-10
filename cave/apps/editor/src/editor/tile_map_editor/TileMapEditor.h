#pragma once
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/view/ViewDesc.h"

#include "engine/private/core/math/geomath.h"
#include "engine/private/runtime/assets/TileMapAsset.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/Tab.h"
#include "editor/widgets/SpriteSelector.h"
#include "editor/services/IPickConsumer.h"

// @TODO: refactor
#include "editor/camera/CameraController.h"

namespace cave {

class AssetRegistry;
class Scene;
class TileMapDocument;

class TileMapEditor final : public Tab,
                            public IPickConsumer,
                            public ISceneTickContributor {
public:
    TileMapEditor(EditorState& editor,
                  DocId doc_id,
                  SceneId preview_scene_id);
    ~TileMapEditor();

    void onCreate() override;
    void onDestroy() override;

    void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) override;
    Option<PickData> GetPickData(const math::Vector2f& pos_screen) override;

    void onInputEvents(const InputFrame& input) override;

    // void DrawMainView(const CameraComponent& p_camera) final;

    // void DrawAssetInspector() final;

    // bool CursorToTile(const Vector2f& p_in, TileIndex& p_out) const;
    DebugId debugId() const override { return debug_id_; }

protected:
    void submitView();

    void drawUIImpl() override;
    void drawMainView(const math::FloatRect& rect);

    void updateRect(math::FloatRect& out_rect);

    // @TODO: refactor
    void TileMapLayerOverview(TileMapAsset& p_tile_map);

    ViewManager& view_manager_;
    const DebugId debug_id_;
    SceneId preview_scene_id_;

    std::unique_ptr<CameraComponent> m_camera;
    ToolBarButtonDesc m_brush_desc;

    SpriteSelector m_sprite_selector;

    // @TODO: move to input controller
    std::unique_ptr<ICameraController> camera_controller_;
    CameraComponent camera_;
    TransformComponent camera_transform_;
    GpuTextureId texture_;
    ViewId view_id_;
};

}  // namespace cave
