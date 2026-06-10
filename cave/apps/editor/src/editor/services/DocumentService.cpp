#include "DocumentService.h"

#include "cave/runtime/framework/IApplication.h"

#include "editor/document/MaterialDocument.h"
#include "editor/document/SceneDocument.h"
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

DocId DocumentService::OpenDoc(const OpenDocDesc& p_desc) {
    DocId doc_id;
    if (auto it = m_doc_cache.find(p_desc.guid); it != m_doc_cache.end()) {
        doc_id = it->second;
    } else {
        auto doc = CreateDoc(m_editor.app(), p_desc);
        doc_id = Base::Create(std::move(doc));
        m_doc_cache[p_desc.guid] = doc_id;
    }

    m_editor.Workspace().RequestOpen(doc_id);
    return doc_id;
}

CloseRequestResult DocumentService::CloseDoc(DocId p_doc_id) {
    IDocument* doc = Resolve(p_doc_id);
    DEV_ASSERT(doc);
    auto handle = doc->GetHandleRaw();
    m_doc_cache.erase(handle.GetGuid());
    Destroy(p_doc_id);
    return {};
}

bool DocumentService::Save(const Guid& p_guid) {
    auto it = m_doc_cache.find(p_guid);
    if (it == m_doc_cache.end()) return false;

    if (IDocument* doc = Resolve(it->second)) {
        return doc->Save();
    }
    return false;
}

}  // namespace cave
