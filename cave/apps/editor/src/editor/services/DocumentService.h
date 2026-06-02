#pragma once
// @TODO: move to public
#include "cave/core/ids/Guid.h"
#include "engine/private/core/ids/GenIdRegistry.h"

#include "editor/document/IDocument.h"

namespace cave {

class EditorState;

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
    DocumentService(EditorState& p_editor);

    DocId OpenDoc(const OpenDocDesc& p_desc);

    CloseRequestResult CloseDoc(DocId p_id);

    IDocument* Resolve(DocId p_id) { return Base::Resolve(p_id); }
    const IDocument* Resolve(DocId p_id) const { return Base::Resolve(p_id); }
    bool IsAlive(DocId p_id) const { return Base::IsAlive(p_id); }

    bool Save(const Guid& p_guid);

private:
    std::unordered_map<Guid, DocId> m_doc_cache;

    EditorState& m_editor;
};

}  // namespace cave