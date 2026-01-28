#include "Tab.h"

#include "editor/EditorState.h"
#include "editor/services/EditService.h"
#include "editor/services/Workspace.h"

// @TODO: refactor
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/renderer/graphics_manager.h"

#include "engine/private/drivers/windows/win32_prerequisites.h"

namespace cave {

enum class CloseDecision {
    Save,
    Discard,
    Cancel,
};

// @TODO: move to Dialog Service
static CloseDecision AskCloseUnsaved(HWND p_owner, const char* p_title) {
    int result = MessageBoxA(
        p_owner,
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
            switch (AskCloseUnsaved(0, "Warning")) {
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

void Tab::DrawUIImpl() {
    ImVec2 top_left(m_rect.Left(), m_rect.Top());
    ImVec2 bottom_right(m_rect.Right(), m_rect.Bottom());

    const auto& gm = *m_editor.GetApp().GetGraphicsManager();
    uint64_t handle = gm.GetFinalImage();
    // add image for drawing
    switch (gm.GetBackend()) {
        case Backend::D3D11:
        case Backend::D3D12: {
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right);
        } break;
        case Backend::OPENGL: {
            ImVec2 uv_min = ImVec2(0, 1);
            ImVec2 uv_max = ImVec2(1, 0);
            if (gm.GetActiveRenderGraphName() == RenderGraphName::PATHTRACER) {
                uv_min = ImVec2(0, 0);
                uv_max = ImVec2(1, 1);
            }
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)handle, top_left, bottom_right, uv_min, uv_max);
        } break;
        case Backend::VULKAN:
        case Backend::METAL: {
        } break;
        default:
            CRASH_NOW();
            break;
    }
}

void Tab::SetTitleAndId(std::string_view p_title, uint32_t p_idx) {
    m_idx = p_idx;
    m_title = p_title.empty() ? "Untitled" : p_title;
    m_window_id = std::format("{}###WorkspaceTab{}", m_title, m_idx);
}

void Tab::Tick(float) {
    const bool dirty = m_editor.EditService().IsDirty(m_doc_id);
    if (dirty) {
        m_flags |= ImGuiWindowFlags_UnsavedDocument;
    } else {
        m_flags &= ~ImGuiWindowFlags_UnsavedDocument;
    }
}

void Tab::OnCreate() {
}

void Tab::OnDestroy() {
}

}  // namespace cave
