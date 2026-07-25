#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/intent/IIntentHandler.h"

#include "editor/document/DocId.h"
#include "editor/edit/IEditCmd.h"

namespace cave {

struct EngineServices;
struct EditorServices;
class SceneCommandWriter;
using SceneCommandWriterFn = std::function<ecs::Entity(SceneCommandWriter&)>;

class EditService final : public IIntentHandler {
public:
    EditService(EngineServices& app_services,
                EditorServices& editor_services);
    ~EditService();

    void submit(DocId doc_id, Owner<IEditCmd>&& cmd);
    void recordApplied(DocId doc_id, Owner<IEditCmd>&& cmd);

    void submit(DocId doc_id, SceneCommandWriterFn&& func);

    void undo(DocId doc_id);
    void redo(DocId doc_id);

    bool canUndo(DocId doc_id) const;
    bool canRedo(DocId doc_id) const;

    bool isDirty(DocId doc_id) const;
    bool save(DocId doc_id);

    bool handleIntent(Intent& intent) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    IDocument* resolve(DocId doc_id);
    const IDocument* resolve(DocId doc_id) const;

    EngineServices& m_app_services;
    EditorServices& m_editor_services;
    const DebugId m_debug_id;
};

}  // namespace cave
