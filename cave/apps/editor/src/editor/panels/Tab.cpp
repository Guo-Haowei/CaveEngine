#include "Tab.h"

#include "editor/EditorState.h"
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
    if (const bool dirty = m_editor.EditService().isDirty(doc_id_)) {
        m_flags |= ImGuiWindowFlags_UnsavedDocument;
    } else {
        m_flags &= ~ImGuiWindowFlags_UnsavedDocument;
    }

    ResetState();
    bool open = true;
    if (ImGui::Begin(windowId(), &open, m_flags)) {
        UpdateState();
        drawUIImpl();
    }
    ImGui::End();

    if (!open) {
        const bool dirty = m_editor.EditService().isDirty(doc_id_);
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
            m_editor.EditService().save(doc_id_);
        }

        m_editor.Workspace().requestClose(doc_id_);
    }
}

void Tab::setTitleAndId(std::string_view title, uint32_t idx) {
    idx_ = idx;
    title_ = title.empty() ? "Untitled" : title;
    window_id_ = std::format("{}###WorkspaceTab{}", title_, idx_);
}

void Tab::onCreate() {
}

void Tab::onDestroy() {
}

}  // namespace cave
