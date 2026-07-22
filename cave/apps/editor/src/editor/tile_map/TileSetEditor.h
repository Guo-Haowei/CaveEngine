#pragma once
#include "cave/runtime/tile_map/TileSetAsset.h"

#include "editor/windows/ViewTabBase.h"
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

    Option<math::Vec2i> worldPointToCell(math::Vec2f point_os,
                                         const TileSetAsset& tile_set) const;

    Assets getAssets() const;

    const DebugId m_debug_id;

    SpriteSelector m_sprite_selector{ SpriteSelector::SelectionMode::Single };
    Option<math::Vec2f> m_cursor;
    Option<uint32_t> m_atlas;
};

}  // namespace cave
