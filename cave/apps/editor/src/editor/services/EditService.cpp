#include "EditService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentBus.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorIntent.h"
#include "editor/EditorState.h"
#include "editor/edit/SceneCommandExecutor_Undo.h"
#include "editor/services/DocumentService.h"
#include "editor/services/SelectionService.h"
#include "editor/services/Workspace.h"

namespace cave {

EditService::EditService(EngineServices& app_services,
                         EditorServices& editor_services)
    : m_app_services(app_services)
    , m_editor_services(editor_services)
    , m_debug_id(MakeDebugId(this)) {
    m_app_services.intentBus().addHandler<EditIntent>(this);
}

EditService::~EditService() {
    m_app_services.intentBus().removeHandler<EditIntent>(this);
}

void EditService::submit(DocId doc_id, Owner<IEditCmd>&& cmd) {
    m_app_services.intentBus().queue<EditIntent>(doc_id, std::move(cmd), false);
}

void EditService::recordApplied(DocId doc_id, Owner<IEditCmd>&& cmd) {
    m_app_services.intentBus().queue<EditIntent>(doc_id, std::move(cmd), true);
}

void EditService::submit(DocId doc_id, SceneCommandWriterFn&& func) {
    SceneRegistry& scene_reg = m_app_services.sceneRegistry();

    Scene* scene = nullptr;
    SceneId scene_id{};
    if (IDocument* doc = m_editor_services.document().resolve(doc_id)) {
        scene_id = doc->previewScene();
        scene = scene_reg.resolve(scene_id);
    }

    if (!scene) {
        return;
    }

    SceneCommandWriter cb(m_app_services.assetRegistry());
    auto ent = func(cb);

    EntityMap map(cb.allocationCount());
    SceneCommandExecutor_Undo executor(scene_reg);
    SceneCommandPlayback::Play(cb, executor, { map, *scene });
    ent = map.resolve(ent);

    if (ent.valid()) {
        SelectionKey selection = {
            .kind = SelectionKind::Entity,
            .doc = doc_id,
            .scene = scene_id,
            .entity = ent,
        };

        m_editor_services.selection().setSelection(doc_id, selection);
    }

    submit(doc_id, std::move(executor.takeCommand()));
}

void EditService::undo(DocId doc_id) {
    if (IDocument* doc = resolve(doc_id)) {
        doc->undo();
    }
}

void EditService::redo(DocId doc_id) {
    if (IDocument* doc = resolve(doc_id)) {
        doc->redo();
    }
}

bool EditService::canUndo(DocId doc_id) const {
    const IDocument* doc = resolve(doc_id);
    return doc ? doc->canUndo() : false;
}

bool EditService::canRedo(DocId doc_id) const {
    const IDocument* doc = resolve(doc_id);
    return doc ? doc->canRedo() : false;
}

bool EditService::isDirty(DocId doc_id) const {
    const IDocument* doc = resolve(doc_id);
    return doc ? doc->isDirty() : false;
}

bool EditService::save(DocId doc_id) {
    IDocument* doc = resolve(doc_id);
    return doc ? doc->save() : false;
}

bool EditService::handleIntent(Intent& intent) {
    if (auto edit_intent = dynamic_cast<EditIntent*>(&intent)) {
        IDocument* doc = resolve(edit_intent->doc_id());
        if (DEV_VERIFY(doc)) {
            if (edit_intent->alreadyExecuted()) {
                doc->recordApplied(std::move(edit_intent->cmd), 0);
            } else {
                doc->apply(std::move(edit_intent->cmd), 0);
            }
        }

        return true;
    }

    return false;
}

IDocument* EditService::resolve(DocId doc_id) {
    return m_editor_services.document().resolve(doc_id);
}

const IDocument* EditService::resolve(DocId doc_id) const {
    return m_editor_services.document().resolve(doc_id);
}

}  // namespace cave
