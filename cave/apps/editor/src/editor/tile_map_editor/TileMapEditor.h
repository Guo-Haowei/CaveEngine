#pragma once
#include "cave/runtime/view/ViewDesc.h"

#include "engine/private/runtime/assets/TileMapAsset.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/ViewTabBase.h"
#include "editor/widgets/SpriteSelector.h"
#include "editor/services/IPickConsumer.h"

namespace cave {

class AssetRegistry;
class Scene;
class TileMapDocument;

class TileMapEditor final : public ViewTabBase,
                            public IPickConsumer {
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
    void submitView();

    void drawUIImpl() override;

    const DebugId debug_id_;

    SpriteSelector sprite_selector_;
};

}  // namespace cave
