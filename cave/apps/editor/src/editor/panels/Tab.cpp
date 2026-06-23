#include "Tab.h"

#include "editor/EditorState.h"
#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

Tab::Tab(EditorState& editor, DocId doc_id)
    : EditorWindow(editor)
    , doc_id_(doc_id) {}

// @TODO: move to Dialog Service
CloseDecision AskCloseUnsaved(const char* title) {
    int result = MessageBoxA(
        NULL,
        "You have unsaved changes.\n\nDo you want to save before closing?",
        title,
        MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON1);

    switch (result) {
        case IDYES:
            return CloseDecision::Save;
        case IDNO:
            return CloseDecision::Discard;
        default:
            return CloseDecision::Cancel;
    }
}

void Tab::drawUI() {
    EditService& edit = editor_services_.edit();
    if (const bool dirty = edit.isDirty(doc_id_)) {
        flags_ |= ImGuiWindowFlags_UnsavedDocument;
    } else {
        flags_ &= ~ImGuiWindowFlags_UnsavedDocument;
    }

    resetState();
    bool open = true;
    if (ImGui::Begin(windowId(), &open, flags_)) {
        updateState();
        drawUIImpl();
    }
    ImGui::End();

    if (!open) {
        const bool dirty = edit.isDirty(doc_id_);
        bool should_save = false;
        if (dirty) {
            switch (AskCloseUnsaved("Warning")) {
                case CloseDecision::Cancel:
                    return;
                case CloseDecision::Save: {
                    should_save = true;
                } break;
                case CloseDecision::Discard: {
                    // Do nothing, close
                } break;
            }
        }
        if (should_save) {
            edit.save(doc_id_);
        }

        editor_services_.workspace().requestClose(doc_id_);
    }
}

const char* Tab::windowId() const {
    IDocument* doc = editor_services_.document().resolve(doc_id_);
    DEV_ASSERT(doc);
    AssetHandle handle = doc->rawHandle();
    const AssetMetaData* meta = handle.meta();
    DEV_ASSERT(meta);

    window_id_ = std::format("{}###WorkspaceTab{}", meta->name, tab_id_.index);
    return window_id_.c_str();
}

void Tab::onCreate() {
}

void Tab::onDestroy() {
}

}  // namespace cave
