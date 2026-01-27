#pragma once
#include "engine/private/runtime/core/GenIdRegistry.h"

#include "editor/document/IDocument.h"

namespace cave {

class EditorState;
class Guid;

struct CreateDocDesc {
    DocKind kind{ DocKind::Scene };
    std::string title;
};

struct CloseRequestResult {
    bool ok{ false };
    bool not_found{ false };
    bool in_use{ false };  // track refcounts/pins
};

class DocumentService : protected GenIdRegistry<IDocument> {
    using Base = GenIdRegistry<IDocument>;

public:
    DocumentService(EditorState& p_editor);

    DocId OpenScene(const Guid& p_guid);

    DocId Create(const CreateDocDesc& p_desc);

    CloseRequestResult Close(DocId p_id);

    IDocument* Resolve(DocId p_id) { return Base::Resolve(p_id); }
    const IDocument* Resolve(DocId p_id) const { return Base::Resolve(p_id); }
    bool IsAlive(DocId p_id) const { return Base::IsAlive(p_id); }

private:
    EditorState& m_editor;
};

}  // namespace cave