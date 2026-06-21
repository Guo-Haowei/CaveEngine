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
    void drawTimeLine(SpriteAnimationAsset& anim, IDocument& doc);
    std::string selectAnimation(SpriteAnimationAsset& anim, std::string_view current_clip);

    void submitView();

    const DebugId debug_id_;

    std::string clip_name_;
    SpriteSelector sprite_selector_{ SpriteSelector::SelectionMode::Multi };

    // const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;
    ToolBarButtonDesc m_play_button;
    ToolBarButtonDesc m_pause_button;

    enum class Request {
        None,
        Play,
        Pause,
    } last_req_{ Request::None };
};

}  // namespace cave
