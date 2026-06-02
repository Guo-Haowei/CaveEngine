#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/intent/IIntentHandler.h"

#include "editor/document/DocId.h"
#include "editor/edit/IEditCmd.h"

namespace cave {

class EditorState;
class SceneCommandWriter;
using SceneCommandWriterFn = std::function<void(SceneCommandWriter&)>;

class EditService final : public IIntentHandler {
public:
    EditService(EditorState& p_editor);
    ~EditService();

    void Submit(DocId p_doc_id, std::unique_ptr<IEditCmd>&& p_cmd);

    void Submit(DocId p_doc_id, SceneCommandWriterFn&& p_func);

    void Undo(DocId p_doc_id);
    void Redo(DocId p_doc_id);

    bool CanUndo(DocId p_doc_id) const;
    bool CanRedo(DocId p_doc_id) const;

    bool IsDirty(DocId p_doc_id) const;
    bool Save(DocId p_doc_id);

    bool HandleIntent(Intent& p_intent) override;

    DebugId GetDebugId() const override { return m_debug_id; }

private:
    IDocument* ResolveDoc(DocId p_doc_id);
    const IDocument* ResolveDoc(DocId p_doc_id) const;

    EditorState& m_editor;

    const DebugId m_debug_id;
};

}  // namespace cave
