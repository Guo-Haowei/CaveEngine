#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/intent/IIntentHandler.h"

#include "editor/document/DocId.h"
#include "editor/edit/IEditCmd.h"

namespace cave {

struct AppServices;
struct EditorServices;
class SceneCommandWriter;
using SceneCommandWriterFn = std::function<void(SceneCommandWriter&)>;

class EditService final : public IIntentHandler {
public:
    EditService(AppServices& app_services,
                EditorServices& editor_services);
    ~EditService();

    void submit(DocId doc_id, std::unique_ptr<IEditCmd>&& cmd);

    void submit(DocId doc_id, SceneCommandWriterFn&& func);

    void undo(DocId doc_id);
    void redo(DocId doc_id);

    bool canUndo(DocId doc_id) const;
    bool canRedo(DocId doc_id) const;

    bool isDirty(DocId doc_id) const;
    bool save(DocId doc_id);

    bool handleIntent(Intent& intent) override;

    DebugId debugId() const override { return debug_id_; }

private:
    IDocument* resolve(DocId doc_id);
    const IDocument* resolve(DocId doc_id) const;

    AppServices& app_services_;
    EditorServices& editor_services_;
    const DebugId debug_id_;
};

}  // namespace cave
