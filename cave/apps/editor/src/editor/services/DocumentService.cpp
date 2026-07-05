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
            return std::make_unique<SceneDocument>(services, desc.guid);
        case AssetType::Material:
            return std::make_unique<MaterialDocument>(services, desc.guid);
        case AssetType::SpriteAnimation:
            return std::make_unique<SpriteAnimationDocument>(services, desc.guid);
        case AssetType::TileMap:
            return std::make_unique<TileMapDocument>(services, desc.guid);
        default:
            return std::make_unique<DocumentBase>(services, desc.guid);
    }
}

DocId DocumentService::openDoc(const OpenDocDesc& desc) {
    DocId doc_id;
    if (auto it = guid_to_doc_.find(desc.guid); it != guid_to_doc_.end()) {
        doc_id = it->second;
    } else {
        auto doc = CreateDoc(engine_services_, desc);
        doc_id = Base::create(std::move(doc));
        guid_to_doc_[desc.guid] = doc_id;
    }

    editor_services_.workspace().requestOpen(doc_id);
    return doc_id;
}

CloseRequestResult DocumentService::closeDoc(DocId doc_id) {
    IDocument* doc = resolve(doc_id);
    DEV_ASSERT(doc);
    auto handle = doc->rawHandle();
    guid_to_doc_.erase(handle.guid());
    destroy(doc_id);
    return {};
}

bool DocumentService::save(const Guid& guid) {
    auto it = guid_to_doc_.find(guid);
    if (it == guid_to_doc_.end()) {
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
