#include "viewer_tab.h"

#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
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

std::shared_ptr<CameraComponent> ViewerTab::CreateDefaultCamera2D() {
    const auto res = DVAR_GET_IVEC2(resolution);
    auto camera = std::make_shared<CameraComponent>();
    camera->SetOrtho();
    camera->SetView2D();
    camera->SetDimension(res.x, res.y);
    camera->SetNear(1.0f);
    camera->SetFar(1000.0f);
    camera->SetPosition(Vector3f(0, 0, 10));
    camera->SetDirty();
    camera->Update();
    return camera;
}

// @TODO: ad 2d 3d flag to camera
std::shared_ptr<CameraComponent> ViewerTab::CreateDefaultCamera3D() {
    const auto res = DVAR_GET_IVEC2(resolution);
    auto camera = std::make_shared<CameraComponent>();
    camera->SetDimension(res.x, res.y);
    camera->SetNear(1.0f);
    camera->SetFar(1000.0f);
    camera->SetPosition(Vector3f(0, 4, 10));
    camera->SetDirty();
    camera->Update();
    return camera;
}

bool ViewerTab::HandleInput(const InputEvent* p_input_event) {
    unused(p_input_event);
    return false;
}

void ViewerTab::DrawMainView() {
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

void ViewerTab::Draw() {
    // @TODO: remove this
    DrawMainView();
}

}  // namespace cave
