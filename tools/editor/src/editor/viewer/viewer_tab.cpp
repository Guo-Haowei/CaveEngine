#include "viewer_tab.h"

#include "engine/renderer/graphics_dvars.h"
#include "engine/scene/scene.h"

#include "editor/editor_layer.h"
#include "editor/viewer/viewer.h"

#include "engine/scene/camera_controller.h"

namespace cave {

// @TODO: refactor
enum {
    CAM_2D,
    CAM_3D,
};

// @TODO: refactor
class EditorCameraController {
public:
    CameraControllerFPS controller_3d;
    CameraController2DEditor controller_2d;

    CameraComponent& GetCamera(int p_idx) { return cameras[p_idx]; }

    std::array<CameraComponent, 2> cameras;
    int current = CAM_3D;

    void Check(bool p_only_2d) {
        if (p_only_2d) {
            current = CAM_2D;
        }
    }

    void Toggle(bool p_only_2d) {
        if (p_only_2d) {
            current = CAM_2D;
        } else {
            current ^= 1;
        }
    }
};

ViewerTab::ViewerTab(EditorLayer& p_editor, Viewer& p_viewer)
    : m_id(TabId::Next())
    , m_editor(p_editor)
    , m_viewer(p_viewer) {

    m_controller = std::make_shared<EditorCameraController>();

    const auto res = DVAR_GET_IVEC2(resolution);
    {
        CameraComponent camera;
        camera.SetDimension(res.x, res.y);
        camera.SetNear(1.0f);
        camera.SetFar(1000.0f);
        camera.SetPosition(Vector3f(0, 4, 10));
        camera.SetDirty();
        camera.Update();

        m_controller->cameras[CAM_3D] = camera;
    }
    {
        CameraComponent camera;
        camera.SetOrtho();
        camera.SetDimension(res.x, res.y);
        camera.SetNear(1.0f);
        camera.SetFar(1000.0f);
        camera.SetPosition(Vector3f(0, 0, 10));
        camera.SetDirty();
        camera.Update();

        m_controller->cameras[CAM_2D] = camera;
    }
}

const CameraComponent& ViewerTab::GetActiveCameraInternal() const {
    return m_controller->cameras[m_controller->current];
}

void ViewerTab::UpdateCamera() {
    // this is for internal update, so const cast is fine
    CameraComponent& camera = GetActiveCamera();

    const auto& input_state = m_viewer.GetInputState();

    const float dt = m_editor.context.timestep;
    const auto& move = input_state.mouse_move;
    const auto& scroll = input_state.scroll;

    const bool only_2d = GetCameraPolicy() == ToolCameraPolicy::Only2D;
    m_controller->Check(only_2d);

    // @TODO: still need to figure out a better way to do it
    switch (m_controller->current) {
        case CAM_2D: {
            CameraInputState state{
                .move = dt * Vector3f(-move.x, move.y, 0.0f),
                .zoomDelta = -dt * scroll,
            };
            m_controller->controller_2d.Update(camera, state);
        } break;
        case CAM_3D: {
            CameraInputState state{
                .move = dt * Vector3f(input_state.dx, input_state.dy, input_state.dz),
                .zoomDelta = dt * scroll,
                .rotation = dt * move,
            };
            m_controller->controller_3d.Update(camera, state);
        } break;
    }

    camera.Update();
}

bool ViewerTab::HandleInput(const std::shared_ptr<InputEvent>& p_input_event) {
    unused(p_input_event);
    return false;
}

void ViewerTab::Draw(Scene*) {
}

}  // namespace cave
