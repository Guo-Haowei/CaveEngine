#include "AssetInspector.h"

#include "editor/services/DocumentService.h"
#include "editor/services/Workspace.h"

namespace cave {

AssetInspector::AssetInspector(EditorState& editor,
                               EditorServices& editor_services)
    : EditorWindow(editor)
    , editor_services_(editor_services) {
}

void AssetInspector::drawUIImpl() {
    DocId doc_id = editor_services_.workspace().focusedDoc();
    IDocument* doc = editor_services_.document().resolve(doc_id);
    if (doc) {
        doc->drawAssetInspector();
    }
}

}  // namespace cave
