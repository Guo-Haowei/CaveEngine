#pragma once
#include "cave/runtime/view/ViewDesc.h"

#include "engine/private/runtime/assets/TileMapAsset.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/ViewTabBase.h"
#include "editor/services/IPickConsumer.h"

namespace cave {

class TileMapEditor final : public ViewTabBase,
                            public IPickConsumer {
    enum class Mode : uint8_t {
        None,
        Painting,
        Erasing,
    };

public:
    TileMapEditor(EditorState& editor,
                  DocId doc_id,
                  SceneId preview_scene_id);

    void onCreate() override;
    void onDestroy() override;

    Option<PickData> getPickData(const math::Vector2f& pos_screen) override;

    void onInputEvents(const InputFrame& input) override;

    DebugId debugId() const override { return debug_id_; }

protected:
    void drawUIImpl() override;

    void submitView();
    void changeMode(Mode mode);
    bool canHandleInput(const InputFrame& input);
    bool updateEditMode(const InputFrame& input);
    void applayEditorTool();

    const DebugId debug_id_;

    Mode mode_{ Mode::None };
    bool lb_down_{ false };
    bool rb_down_{ false };
    math::Vector2f cursor_;
};

}  // namespace cave
