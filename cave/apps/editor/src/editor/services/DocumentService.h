#pragma once
#include "cave/core/ids/Guid.h"

// @TODO: move to public
#include "engine/private/core/ids/GenIdRegistry.h"

#include "editor/document/IDocument.h"

namespace cave {

struct EngineServices;
struct EditorServices;

struct CloseRequestResult {
    bool ok{ false };
    bool not_found{ false };
    bool in_use{ false };  // track refcounts/pins
};

struct OpenDocDesc {
    Guid guid;
    AssetType asset_type;
    bool focused{ true };
};

class DocumentService : protected GenIdRegistry<IDocument> {
    using Base = GenIdRegistry<IDocument>;

public:
    DocumentService(EngineServices& app_services,
                    EditorServices& editor_services) noexcept
        : app_services_(app_services)
        , editor_services_(editor_services) {}

    DocId openDoc(const OpenDocDesc& desc);

    CloseRequestResult closeDoc(DocId doc_id);

    bool isAlive(DocId doc_id) const { return Base::isAlive(doc_id); }

    IDocument* resolve(DocId doc_id) { return Base::resolve(doc_id); }
    const IDocument* resolve(DocId doc_id) const { return Base::resolve(doc_id); }

    bool save(DocId doc_id);
    bool save(const Guid& guid);

private:
    EngineServices& app_services_;
    EditorServices& editor_services_;

    std::unordered_map<Guid, DocId> guid_to_doc_;
};

}  // namespace cave