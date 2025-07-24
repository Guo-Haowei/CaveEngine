#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/sprite_animation_asset.h"
#include "engine/ecs/entity.h"

#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/sprite_selector.h"

namespace cave {

class AssetRegistry;
class Document;
class InputEvent;
class TileMapDocument;
class Viewer;

using SpriteAnimationDocument = Document;

class SpriteAnimationEditor : public ViewerTab {
public:
    SpriteAnimationEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) override;

    void OnCreate(const Guid& p_guid) override;

    void OnDestroy() override;

    void OnActivate() override;

    void DrawMainView(const CameraComponent& p_camera) override;

    void DrawAssetInspector() override;

    Document& GetDocument() const override;

    Scene* GetScene() override { return m_tmp_scene.get(); }

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    const std::vector<ToolBarButtonDesc>& GetToolBarButtons() const override;

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
};

}  // namespace cave
