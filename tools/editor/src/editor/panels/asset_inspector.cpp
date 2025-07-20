#include "asset_inspector.h"

#include "editor/editor_layer.h"
#include "editor/viewer/viewer.h"
#include "editor/viewer/viewer_tab.h"

namespace cave {

AssetInspector::AssetInspector(EditorLayer& p_editor)
    : EditorWindow(p_editor) {
}

void AssetInspector::OnAttach() {
}

void AssetInspector::UpdateInternal(Scene*) {
    if (ViewerTab* tab = m_editor.GetViewer().GetActiveTab(); tab) {
        tab->DrawAssetInspector();
    }
}

}  // namespace cave
