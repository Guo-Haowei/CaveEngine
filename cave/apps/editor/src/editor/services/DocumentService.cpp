#include "DocumentService.h"

#include "editor/document/MaterialDocument.h"
#include "editor/document/SceneDocument.h"

#include "cave/runtime/framework/IApplication.h"

#include "editor/EditorState.h"
#include "editor/services/Workspace.h"

namespace cave {

DocumentService::DocumentService(EditorState& p_editor)
    : m_editor(p_editor) {
}

static std::unique_ptr<IDocument> CreateDoc(IApplication& p_app, const OpenDocDesc& p_desc) {
    switch (p_desc.asset_type) {
        case AssetType::Scene:
            return std::make_unique<SceneDocument>(p_app, p_desc.guid);
        case AssetType::Material:
            return std::make_unique<MaterialDocument>(p_app, p_desc.guid);
        default:
            return std::make_unique<DocumentBase>(p_app, p_desc.guid);
    }
}

DocId DocumentService::OpenDoc(OpenDocDesc p_desc) {
    DocId doc_id;
    if (auto it = m_doc_cache.find(p_desc.guid); it != m_doc_cache.end()) {
        doc_id = it->second;
    } else {
        auto doc = CreateDoc(m_editor.GetApp(), p_desc);
        doc_id = Base::Create(std::move(doc));
        m_doc_cache[p_desc.guid] = doc_id;
    }

    m_editor.Workspace().Submit(WorkspaceRequest::OpenDoc(doc_id));
    return doc_id;
}

CloseRequestResult DocumentService::Close(DocId p_id) {
    unused(p_id);
    return {};
}

}  // namespace cave
