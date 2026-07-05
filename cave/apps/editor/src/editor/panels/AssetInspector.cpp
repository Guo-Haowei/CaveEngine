#include "AssetInspector.h"

#include "editor/services/EditorServices.h"
#include "editor/services/DocumentService.h"
#include "editor/services/Workspace.h"

namespace cave {

using namespace ::cave::math;

AssetInspector::AssetInspector(EditorState& editor)
    : EditorWindow(editor) {
}

void AssetInspector::onAttach() {
}

void AssetInspector::drawUIImpl() {
    Workspace& workspace = m_editor_services.workspace();
    DocId doc_id = workspace.focusedDoc();
    IDocument* doc = m_editor_services.document().resolve(doc_id);
    if (doc == nullptr) {
        return;
    }

    if (Tab* tab = workspace.focusedTab()) {
        tab->drawAssetInspector(*doc);
    }
}

}  // namespace cave
