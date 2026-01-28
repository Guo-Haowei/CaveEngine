#include "Tab.h"

#include "editor/EditorState.h"
#include "editor/services/EditService.h"

// @TODO: refactor
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/renderer/graphics_manager.h"

namespace cave {

Tab::Tab(EditorState& p_editor, DocId p_doc_id)
    : EditorWindow(p_editor)
    , m_doc_id(p_doc_id) {}

void Tab::DrawUIImpl() {
    // ImGui::Text("%s", m_window_id.c_str());
    // ImGui::Text("ID: %u", m_idx);

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
