#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"

#include "editor/document/DocId.h"
#include "editor/edit/IEditCmd.h"

namespace cave {

class EditorState;

enum class EntityType : uint8_t;

class EditService {
public:
    EditService(EditorState& p_editor);

    void Submit(DocId p_doc_id, std::unique_ptr<IEditCmd> p_cmd);

    void Undo(DocId p_doc_id);
    void Redo(DocId p_doc_id);

    bool CanUndo(DocId p_doc_id) const;
    bool CanRedo(DocId p_doc_id) const;

    bool IsDirty(DocId p_doc_id) const;
    bool Save(DocId p_doc_id);

    void FlushPendingCmds();

private:
    IDocument* ResolveDoc(DocId p_doc_id);
    const IDocument* ResolveDoc(DocId p_doc_id) const;

    EditorState& m_editor;
    std::unordered_map<DocId, std::vector<std::unique_ptr<IEditCmd>>> m_pending_cmds;
};

}  // namespace cave
