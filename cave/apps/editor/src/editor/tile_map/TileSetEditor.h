#pragma once
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/panels/ViewTabBase.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

struct ImageAsset;

class TileSetEditor final : public ViewTabBase {
public:
    TileSetEditor(EditorState& editor,
                  DocId doc_id,
                  SceneId scene_id);

    ~TileSetEditor() override;

    DebugId debugId() const override { return m_debug_id; }

private:
    struct Assets {
        ImageAsset* image{};
        TileSetAsset* tile_set{};
    };

    void onCreate() override;
    void onDestroy() override;

    void submitView();

    void drawUIImpl() override;

    void onInputEvents(const InputFrame& input) override;

    void drawTiles();
    void drawAssetInspector(IDocument& doc) override;

    Assets getAssets() const;

    const DebugId m_debug_id;

    SpriteSelector m_sprite_selector{ SpriteSelector::SelectionMode::Single };
};

}  // namespace cave
