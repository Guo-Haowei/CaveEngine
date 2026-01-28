#include "ViewerTab.h"

#include "engine/private/runtime/string/StringUtils.h"
#include "engine/private/renderer/graphics_dvars.h"
#include "engine/private/renderer/graphics_manager.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/framework/InputSystem.h"
#include "engine/private/runtime/framework/RuntimeHost.h"
#include "engine/private/runtime/scene/EntityFactory.h"

#include "editor/EditorState.h"
#include "editor/panels/AssetInspector.h"
#include "editor/viewer/Viewer.h"

// @TODO: refactor
#include "engine/private/runtime/scene/ISceneRegistry.h"

namespace cave {

#if 0
void ViewerTab::OnActivate() {
    m_active = true;
    OnActivateInternal();

    IApplication& app = m_editor.GetApp();

    app.GetInputSystem()->Router().Register(this);
    app.GetSceneScheduler().Register(this);
}

void ViewerTab::OnDeactivate() {
    IApplication& app = m_editor.GetApp();

    app.GetSceneScheduler().Unregister(this);
    app.GetInputSystem()->Router().Unregister(this);

    OnDeactivateInternal();
    m_active = false;
}

void ViewerTab::DrawAssetInspector() {
    m_editor.GetAssetInspector().DrawContentBrowser();
}

void ViewerTab::DrawMainView(const CameraComponent&) {
    const auto canvas_min = m_viewer.GetCanvasMin();
    const auto canvas_max = canvas_min + m_viewer.GetCanvasSize();

    ImVec2 top_left(canvas_min.x, canvas_min.y);
    ImVec2 bottom_right(canvas_max.x, canvas_max.y);

    // @TODO: fix this
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

void ViewerTab::Update(float p_timestep) {
    m_camera_state.move *= p_timestep;
    m_camera_state.zoom_delta *= p_timestep;
    m_camera_state.rotation *= p_timestep;

    m_camera_controller->Update(m_camera_state);
}

void ViewerTab::OnEvents(const std::vector<InputEvent>& p_events) {
    if (!m_viewer.IsHovered()) {
        return;
    }

    if (m_editor.IsPlaying()) {
        return;
    }

    const KeyState& st = m_editor.GetApp().GetInputSystem()->GetKeyState();
    if (st.AnyAltDown() || st.AnyCtrlDown() || st.AnyShiftDown()) {
        m_camera_state = {};
        return;
    }

    switch (m_dimension) {
        case DIMENSION_2: {
            m_camera_state = CreateCameraInputState2D(p_events, st);
        } break;
        case DIMENSION_3: {
            m_camera_state = CreateCameraInputState3D(p_events, st);
        } break;
        default: {
            CRASH_NOW_MSG("invalid dimension");
        } break;
    }
}
#endif

}  // namespace cave
