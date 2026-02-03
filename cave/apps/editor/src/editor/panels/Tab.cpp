#include "Tab.h"

#include "editor/EditorState.h"
#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

// @TODO: move to Dialog Service
CloseDecision AskCloseUnsaved(const char* p_title) {
    int result = MessageBoxA(
        NULL,
        "You have unsaved changes.\n\nDo you want to save before closing?",
        p_title,
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

Tab::Tab(EditorState& p_editor, DocId p_doc_id)
    : EditorWindow(p_editor)
    , m_doc_id(p_doc_id) {}

void Tab::DrawUI() {
    if (const bool dirty = m_editor.EditService().IsDirty(m_doc_id)) {
        m_flags |= ImGuiWindowFlags_UnsavedDocument;
    } else {
        m_flags &= ~ImGuiWindowFlags_UnsavedDocument;
    }

    ResetState();
    bool open = true;
    if (ImGui::Begin(GetWindowId(), &open, m_flags)) {
        UpdateState();
        DrawUIImpl();
    }
    ImGui::End();

    if (!open) {
        const bool dirty = m_editor.EditService().IsDirty(m_doc_id);
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
            m_editor.EditService().Save(m_doc_id);
        }
        m_editor.Workspace().Submit(WorkspaceRequest::Close(m_doc_id));
    }
}

void Tab::SetTitleAndId(std::string_view p_title, uint32_t p_idx) {
    m_idx = p_idx;
    m_title = p_title.empty() ? "Untitled" : p_title;
    m_window_id = std::format("{}###WorkspaceTab{}", m_title, m_idx);
}

void Tab::Tick(float) {
}

void Tab::OnCreate() {
}

void Tab::OnDestroy() {
}

}  // namespace cave
