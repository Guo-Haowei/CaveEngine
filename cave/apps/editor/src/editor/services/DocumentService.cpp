#include "DocumentService.h"

#include "cave/runtime/framework/EngineServices.h"

#include "editor/animation_editor/SpriteAnimationDocument.h"
#include "editor/document/MaterialDocument.h"
#include "editor/document/TileMapDocument.h"
#include "editor/document/SceneDocument.h"
#include "editor/services/EditorServices.h"
#include "editor/services/Workspace.h"

namespace cave {

static std::unique_ptr<IDocument> CreateDoc(EngineServices& services, const OpenDocDesc& desc) {
    switch (desc.asset_type) {
        case AssetType::Scene:
            return MakeOwner<SceneDocument>(services, desc.guid);
        case AssetType::Prefab:
            return MakeOwner<SceneDocument>(services, desc.guid);
        case AssetType::Material:
            return MakeOwner<MaterialDocument>(services, desc.guid);
        case AssetType::SpriteAnimation:
            return MakeOwner<SpriteAnimationDocument>(services, desc.guid);
        case AssetType::TileMap:
            return MakeOwner<TileMapDocument>(services, desc.guid);
        default:
            return MakeOwner<DocumentBase>(services, desc.guid);
    }
}

DocId DocumentService::openDoc(const OpenDocDesc& desc) {
    DocId doc_id;
    if (auto it = m_guid_to_doc.find(desc.guid); it != m_guid_to_doc.end()) {
        doc_id = it->second;
    } else {
        auto doc = CreateDoc(m_engine_services, desc);
        doc_id = Base::create(std::move(doc));
        m_guid_to_doc[desc.guid] = doc_id;
    }

    m_editor_services.workspace().requestOpen(doc_id);
    return doc_id;
}

CloseRequestResult DocumentService::closeDoc(DocId doc_id) {
    IDocument* doc = resolve(doc_id);
    DEV_ASSERT(doc);
    auto handle = doc->rawHandle();
    m_guid_to_doc.erase(handle.guid());
    destroy(doc_id);
    return {};
}

bool DocumentService::save(const Guid& guid) {
    auto it = m_guid_to_doc.find(guid);
    if (it == m_guid_to_doc.end()) {
        return false;
    }

    return save(it->second);
}

bool DocumentService::save(DocId doc_id) {
    if (IDocument* doc = resolve(doc_id)) {
        if (doc->save()) {
            doc->markSaved();
            return true;
        }
    }
    return false;
}

}  // namespace cave
