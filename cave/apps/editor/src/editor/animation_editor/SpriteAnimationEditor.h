#pragma once
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/Entity.h"

#include "editor/panels/ViewTabBase.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

// @TODO: better editor
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

    std::string selected_clip_;
    SpriteSelector sprite_selector_{ SpriteSelector::SelectionMode::Multi };

    ToolBarButtonDesc play_button_;
    ToolBarButtonDesc pause_button_;

    enum class Request {
        None,
        Play,
        Pause,
    } last_req_{ Request::None };
};

}  // namespace cave
