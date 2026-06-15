#pragma once
#include "cave/runtime/assets/AssetHandle.h"

#include "editor/panels/Tab.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

class TileSetEditor : public Tab {
public:
    TileSetEditor(EditorState& editor,
                  DocId doc_id);

    ~TileSetEditor();

    void onCreate() override;
    void onDestroy() override;
};

}  // namespace cave
