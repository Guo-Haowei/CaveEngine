#include "EditService.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/core/diagnostics/Profiler.h"
#include "cave/runtime/framework/IApplication.h"
#include "cave/runtime/intent/IntentDispatcher.h"
#include "cave/runtime/scene/SceneCommandPlayback.h"
#include "cave/runtime/scene/SceneCommandWriter.h"

#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorIntent.h"
#include "editor/EditorState.h"
#include "editor/edit/SceneCommandExecutor_Undo.h"
#include "editor/services/DocumentService.h"
#include "editor/services/Workspace.h"

namespace cave {

EditService::EditService(EditorState& p_editor)
    : m_editor(p_editor)
    , m_debug_id(MakeDebugId(this)) {
    m_editor.app().IntentDispatcher()->AddHandler<EditIntent>(this);
}

EditService::~EditService() {
    m_editor.app().IntentDispatcher()->RemoveHandler<EditIntent>(this);
}

void EditService::Submit(DocId p_doc_id, std::unique_ptr<IEditCmd>&& p_cmd) {
    m_editor.app().IntentDispatcher()->Queue<EditIntent>(p_doc_id, std::move(p_cmd));
}

void EditService::Submit(DocId p_doc_id, SceneCommandWriterFn&& p_func) {
    IApplication& p_app = m_editor.app();

    SceneRegistry& p_scene_reg = *p_app.GetSceneRegistry();

    Scene* scene = nullptr;
    if (IDocument* doc = m_editor.DocumentService().Resolve(p_doc_id)) {
        SceneId scene_id = doc->GetPreviewScene();
        scene = p_scene_reg.Resolve(scene_id);
    }

    if (!scene) {
        return;
    }

    SceneCommandWriter cb(*p_app.GetAssetRegistry());
    p_func(cb);

    EntityMap map(cb.GetAllocationCount());
    SceneCommandExecutor_Undo executor(*p_app.GetSceneRegistry());
    SceneCommandPlayback::Play(cb, executor, { map, *scene });

    Submit(p_doc_id, std::move(executor.MoveCommand()));
}

void EditService::Undo(DocId p_doc_id) {
    if (IDocument* doc = ResolveDoc(p_doc_id)) {
        doc->Undo();
    }
}

void EditService::Redo(DocId p_doc_id) {
    if (IDocument* doc = ResolveDoc(p_doc_id)) {
        doc->Redo();
    }
}

bool EditService::CanUndo(DocId p_doc_id) const {
    if (const IDocument* doc = ResolveDoc(p_doc_id)) {
        return doc->CanUndo();
    }

    return false;
}

bool EditService::CanRedo(DocId p_doc_id) const {
    if (const IDocument* doc = ResolveDoc(p_doc_id)) {
        return doc->CanRedo();
    }

    return false;
}

bool EditService::IsDirty(DocId p_doc_id) const {
    if (const IDocument* doc = ResolveDoc(p_doc_id)) {
        return doc->IsDirty();
    }

    return false;
}

bool EditService::Save(DocId p_doc_id) {
    if (IDocument* doc = ResolveDoc(p_doc_id)) {
        return doc->Save();
    }

    return false;
}

bool EditService::handleIntent(Intent& p_intent) {
    if (auto intent = dynamic_cast<EditIntent*>(&p_intent)) {
        IDocument* doc = ResolveDoc(intent->doc_id);
        if (DEV_VERIFY(doc)) {
            doc->Apply(std::move(intent->cmd), 0);
        }

        return true;
    }

    return false;
}

const IDocument* EditService::ResolveDoc(DocId p_doc_id) const {
    return m_editor.DocumentService().Resolve(p_doc_id);
}

IDocument* EditService::ResolveDoc(DocId p_doc_id) {
    return m_editor.DocumentService().Resolve(p_doc_id);
}

}  // namespace cave
