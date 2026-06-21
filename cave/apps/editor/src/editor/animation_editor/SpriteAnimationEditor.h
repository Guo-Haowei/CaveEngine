#pragma once
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/Entity.h"

#include "editor/panels/ViewTabBase.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

class SpriteAnimationAsset;
struct ImageAsset;

class SpriteAnimationEditor final : public ViewTabBase {
public:
    SpriteAnimationEditor(EditorState& editor,
                          DocId doc_id,
                          SceneId preview_scene_id);

    void onCreate() override;
    void onDestroy() override;

    DebugId debugId() const override { return debug_id_; }

private:
    void drawUIImpl() override;
    void drawAssetInspector(IDocument& doc) override;

    void drawFrameSelector(SpriteAnimationAsset& anim, ImageAsset& image_asset);
    void drawTimeLine(SpriteAnimationAsset& anim);

    void submitView();

    const DebugId debug_id_;

    std::string clip_name_;
    SpriteSelector sprite_selector_{ SpriteSelector::SelectionMode::Multi };
};

#if 0
class SpriteAnimationEditor {
public:
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

    ToolBarButtonDesc m_play_button;
    ToolBarButtonDesc m_pause_button;
};
#endif

}  // namespace cave
