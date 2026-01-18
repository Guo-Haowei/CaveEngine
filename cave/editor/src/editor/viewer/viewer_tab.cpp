#include "viewer_tab.h"

#include "engine/core/string/string_utils.h"
#include "engine/renderer/graphics_dvars.h"
#include "engine/renderer/graphics_manager.h"
#include "engine/runtime/asset_registry.h"
#include "engine/scene/scene.h"

#include "editor/document/document.h"
#include "editor/editor_layer.h"
#include "editor/panels/asset_inspector.h"
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

void ViewerTab::DrawAssetInspector() {
    m_editor.GetAssetInspector().DrawContentBrowser();
}

void ViewerTab::DrawMainView(const CameraComponent&) {
    const auto canvas_min = m_viewer.GetCanvasMin();
    const auto canvas_max = canvas_min + m_viewer.GetCanvasSize();

    ImVec2 top_left(canvas_min.x, canvas_min.y);
    ImVec2 bottom_right(canvas_max.x, canvas_max.y);

    // @TODO: fix this
    const auto& gm = *m_editor.GetApplication()->GetGraphicsManager();
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

// @TODO: refactor this part
struct InputCache {
    int dx, dy, dz;
    float scroll;
    Vector2f mouse_move;
    std::array<bool, 3> buttons;

    InputCache() { Reset(); }

    void Reset() {
        dx = dy = dz = 0;
        scroll = 0.0f;
        mouse_move = Vector2f{ 0, 0 };
        buttons.fill(0);
    }
};

void ViewerTab::Update(float p_timestep,
                       const ViewportInput& p_input,
                       bool p_focused) {
    if (!p_focused) {
    }

    InputCache c;

    bool handled = false;

    if (p_input.keys.test(std::to_underlying(KeyCode::KEY_D))) {
        ++c.dx;
        handled = true;
    }
    if (p_input.keys.test(std::to_underlying(KeyCode::KEY_A))) {
        --c.dx;
        handled = true;
    }
    if (p_input.keys.test(std::to_underlying(KeyCode::KEY_E))) {
        ++c.dy;
        handled = true;
    }
    if (p_input.keys.test(std::to_underlying(KeyCode::KEY_Q))) {
        --c.dy;
        handled = true;
    }
    if (p_input.keys.test(std::to_underlying(KeyCode::KEY_W))) {
        ++c.dz;
        handled = true;
    }
    if (p_input.keys.test(std::to_underlying(KeyCode::KEY_S))) {
        --c.dz;
        handled = true;
    }

    if (p_input.wheel_delta != 0) {
        c.scroll += 3.0f * p_input.wheel_delta;
    }

    if (p_input.buttons.test(std::to_underlying(MouseButton::MIDDLE))) {
        c.mouse_move += p_input.mouse_move;
    }

    CameraInputState state{
        .move = p_timestep * Vector3f(c.dx, c.dy, c.dz),
        .zoomDelta = p_timestep * c.scroll,
        .rotation = p_timestep * c.mouse_move,
    };

    CameraControllerFPS m_controller_3d;
    CameraComponent& camera = const_cast<CameraComponent&>(GetActiveCameraInternal());
    m_controller_3d.Update(camera, state);
    camera.Update();
}

void ViewerTab::BuildViews(std::vector<SceneView>& p_out_views, bool p_is_opengl) {
    // @TODO: refactor this part
    const CameraComponent& camera = GetActiveCameraInternal();

    SceneView scene_view;
    scene_view.scene = GetScene();
    ViewInfo::FromCamera(camera, scene_view.view_info, p_is_opengl);

    p_out_views.push_back(scene_view);
}

}  // namespace cave
