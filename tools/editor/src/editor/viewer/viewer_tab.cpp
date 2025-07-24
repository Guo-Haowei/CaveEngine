#include "viewer_tab.h"

#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"

#include "editor/document/document.h"
#include "editor/editor_layer.h"
#include "editor/viewer/viewer.h"

namespace cave {

const Guid& ViewerTab::GetGuid() const {
    return GetDocument().GetGuid();
}

ViewerTab::ViewerTab(EditorLayer& p_editor, Viewer& p_viewer)
    : m_id(TabId::Next())
    , m_editor(p_editor)
    , m_viewer(p_viewer) {
}

void ViewerTab::SelectEntity(ecs::Entity p_selected) {
    m_selected = p_selected;
    if (Scene* scene = GetScene(); scene) {
        scene->m_selected = m_selected;
    }
}

void ViewerTab::OnCreate(const Guid& p_guid) {
    auto handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();
    auto meta = handle.GetMeta();
    DEV_ASSERT(meta);

    m_title = std::format("{}###{}", meta->name, handle.GetGuid().ToString());

    LOG_OK("ViewerTab '{}' created", m_title);
}

void ViewerTab::CreateDefaultCamera2D(CameraComponent& p_out) {
    const auto res = DVAR_GET_IVEC2(resolution);
    p_out.SetOrthoFlag();
    p_out.SetView2dFlag();
    p_out.SetDimension(res.x, res.y);
    p_out.SetNear(1.0f);
    p_out.SetFar(1000.0f);
    p_out.SetPosition(Vector3f(0, 0, 10));
    p_out.SetDirtyFlag();
    p_out.Update();
}

void ViewerTab::CreateDefaultCamera3D(CameraComponent& p_out) {
    const auto res = DVAR_GET_IVEC2(resolution);
    auto camera = std::make_shared<CameraComponent>();
    p_out.SetDimension(res.x, res.y);
    p_out.SetNear(1.0f);
    p_out.SetFar(1000.0f);
    p_out.SetPosition(Vector3f(0, 4, 10));
    p_out.SetDirtyFlag();
    p_out.Update();
}

void ViewerTab::DrawToolBar() {
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    auto& colors = ImGui::GetStyle().Colors;
    const auto& button_hovered = colors[ImGuiCol_ButtonHovered];
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(button_hovered.x, button_hovered.y, button_hovered.z, 0.5f));
    const auto& button_active = colors[ImGuiCol_ButtonActive];
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(button_active.x, button_active.y, button_active.z, 0.5f));

    const auto buttons = GetToolBarButtons();

    for (size_t i = 0; i < buttons.size(); ++i) {
        const ToolBarButtonDesc& desc = buttons[i];
        if (i != 0) ImGui::SameLine();
        DrawToolBarButton(desc);
    }

    // ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

void ViewerTab::DrawMainView(const CameraComponent&) {
    const auto canvas_min = m_viewer.GetCanvasMin();
    const auto canvas_max = canvas_min + m_viewer.GetCanvasSize();

    ImVec2 top_left(canvas_min.x, canvas_min.y);
    ImVec2 bottom_right(canvas_max.x, canvas_max.y);

    // @TODO: fix this
    const auto& gm = IGraphicsManager::GetSingleton();
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

}  // namespace cave
