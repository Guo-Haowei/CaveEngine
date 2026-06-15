#include "TileSetEditor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "cave/runtime/ecs/components/CameraComponent.h"

#include "engine/private/runtime/assets/ImageAsset.h"
#include "engine/private/runtime/assets/TileSetAsset.h"
#include "engine/private/ui/layout.h"

namespace cave {

TileSetEditor::TileSetEditor(EditorState& editor,
                             DocId doc_id)
    : Tab(editor, doc_id) {
}

TileSetEditor::~TileSetEditor() = default;

void TileSetEditor::onCreate() {

}

void TileSetEditor::onDestroy() {

}

}  // namespace cave
