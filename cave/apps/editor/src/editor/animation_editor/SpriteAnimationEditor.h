#pragma once
#include "engine/private/assets/asset_handle.h"
#include "engine/private/assets/sprite_animation_asset.h"
#include "engine/private/ecs/entity.h"

#include "editor/viewer/ViewerTab.h"
#include "editor/widgets/SpriteSelector.h"
#include "editor/widgets/ToolBar.h"

namespace cave {

class AssetRegistry;
class OldDocument;
class TileMapDocument;
class Viewer;

using SpriteAnimationDocument = OldDocument;

class SpriteAnimationEditor : public ViewerTab {
public:
    SpriteAnimationEditor(EditorState& p_editor, Viewer& p_viewer);

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    void DrawAssetInspector() final;

    OldDocument& GetDocument() const final;

protected:
    void OnCreateInternal(const Guid& p_guid) final;

    void OnActivateInternal() final;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    void DrawFrameSelector(ImageAsset& p_image_asset);
    void DrawTimeLine();
    void ImageSourceDropTarget();

    AssetRegistry* m_asset_registry = nullptr;

    std::shared_ptr<Scene> m_tmp_scene;

    std::unique_ptr<CameraComponent> m_camera;

    std::shared_ptr<SpriteAnimationDocument> m_document;

    SpriteSelector m_sprite_selector;

    std::string m_clip_name;

    ecs::Entity m_animator_id;

    ToolBarButtonDesc m_play_button;
    ToolBarButtonDesc m_pause_button;
};

}  // namespace cave
