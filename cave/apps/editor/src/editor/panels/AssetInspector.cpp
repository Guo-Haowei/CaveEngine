#include "AssetInspector.h"

#include "editor/services/DocumentService.h"
#include "editor/services/Workspace.h"

namespace cave {

using namespace ::cave::math;

AssetInspector::AssetInspector(EditorState& editor,
                               EditorServices& editor_services)
    : EditorWindow(editor)
    , editor_services_(editor_services) {
}

void AssetInspector::onAttach() {
}

void AssetInspector::drawUIImpl() {
    DocId doc_id = editor_services_.workspace().focusedDoc();
    IDocument* doc = editor_services_.document().resolve(doc_id);
    if (doc == nullptr) {
        return;
    }

    if (Tab* tab = editor_services_.workspace().focusedTab()) {
        tab->drawAssetInspector(*doc);
    }
}

}  // namespace cave
