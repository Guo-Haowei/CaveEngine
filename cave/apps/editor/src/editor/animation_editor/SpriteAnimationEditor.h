#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/runtime/assets/AssetHandle.h"

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

    DebugId debugId() const override { return m_debug_id; }

private:
    void drawUIImpl() override;
    void drawAssetInspector(IDocument& doc) override;

    void drawFrameSelector(SpriteAnimationAsset& anim, ImageAsset& image_asset);
    void drawTimeLine(SpriteAnimationAsset& anim, IDocument& doc);
    std::string selectAnimation(SpriteAnimationAsset& anim, std::string_view current_clip);

    void submitView();

    const DebugId m_debug_id;

    std::string m_text_buffer;
    SpriteSelector m_sprite_selector{ SpriteSelector::SelectionMode::Multi };

    ToolBarButtonDesc m_play_button;
    ToolBarButtonDesc m_pause_button;

    enum class Request {
        None,
        Play,
        Pause,
    } m_last_req{ Request::None };
};

}  // namespace cave
