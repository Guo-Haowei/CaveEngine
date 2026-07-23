#pragma once
#include "cave/core/ids/GenIdRegistry.h"
#include "cave/core/ids/Guid.h"

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
        : m_engine_services(app_services)
        , m_editor_services(editor_services) {}

    DocId loadDoc(const OpenDocDesc& desc);
    DocId openDoc(const OpenDocDesc& desc);

    CloseRequestResult closeDoc(DocId doc_id);

    bool isAlive(DocId doc_id) const { return Base::isAlive(doc_id); }

    IDocument* resolve(DocId doc_id) { return Base::resolve(doc_id); }
    const IDocument* resolve(DocId doc_id) const { return Base::resolve(doc_id); }

    bool markDirty(DocId doc_id);

    bool save(DocId doc_id);
    bool save(const Guid& guid);
    void saveAll();

    bool onCloseRequested();

private:
    EngineServices& m_engine_services;
    EditorServices& m_editor_services;

    std::unordered_map<Guid, DocId> m_guid_to_doc;
};

}  // namespace cave