#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/sprite_animation_asset.h"
#include "engine/ecs/entity.h"

#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/sprite_selector.h"
#include "editor/widgets/tool_bar.h"

namespace cave {

class AssetRegistry;
class Document;
class InputEvent;
class TileMapDocument;
class Viewer;

using SpriteAnimationDocument = Document;

class SpriteAnimationEditor : public ViewerTab {
public:
    SpriteAnimationEditor(EditorState& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) final;

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    void DrawAssetInspector() final;

    Document& GetDocument() const final;

    Scene* GetScene() final {
        return m_tmp_scene.get();
    }

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
