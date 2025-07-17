#pragma once
#include "engine/input/input_router.h"
#include "engine/scene/camera_controller.h"
#include "editor/editor_window.h"
#include "editor/enums.h"

namespace cave {

class ViewerTab;

class Viewer : public EditorWindow, public IInputHandler {
public:
    Viewer(EditorLayer& p_editor);

    HandleInputResult HandleInput(std::shared_ptr<InputEvent> p_input_event) override;

    std::optional<Vector2f> CursorToNDC(Vector2f p_point) const;

    const Vector2f& GetCanvasMin() const { return m_canvas_min; }
    const Vector2f& GetCanvasSize() const { return m_canvas_size; }

    CameraComponent& GetActiveCamera() { return m_controller.cameras[m_controller.current]; }

    auto& GetInputState() { return m_input_state; }

    void OpenTool(AssetEditorType p_type, const Guid& p_guid);
    ViewerTab* GetActiveTool();

protected:
    void UpdateInternal(Scene* p_scene) override;
    void UpdateTab(Scene* p_scene);

    void UpdateCamera();

    void DrawToolBar();
    void DrawGui(Scene* p_scene);

    void UpdateFrameSize();
    HandleInputResult HandleInputCamera(std::shared_ptr<InputEvent> p_input_event);

    Vector2f m_canvas_min;
    Vector2f m_canvas_size;
    bool m_focused;

    struct InputState {
        int dx, dy, dz;
        float scroll;
        Vector2f mouse_move;
        std::optional<Vector2f> ndc;
        std::array<bool, 3> buttons;

        InputState() { Reset(); }

        void Reset() {
            dx = dy = dz = 0;
            scroll = 0.0f;
            mouse_move = Vector2f{ 0, 0 };
            ndc = std::nullopt;
            buttons.fill(0);
        }
    } m_input_state;

    // @TODO: refactor
    enum {
        CAM2D,
        CAM3D,
    };
    struct EditorCameraController {
        CameraControllerFPS controller_3d;
        CameraController2DEditor controller_2d;

        CameraComponent& GetCamera(int p_idx) { return cameras[p_idx]; }

        std::array<CameraComponent, 2> cameras;
        int current = CAM3D;

        void Check(bool p_only_2d) {
            if (p_only_2d) {
                current = CAM2D;
            }
        }

        void Toggle(bool p_only_2d) {
            if (p_only_2d) {
                current = CAM2D;
            } else {
                current ^= 1;
            }
        }
    } m_controller;

    std::array<std::unique_ptr<ViewerTab>, std::to_underlying(AssetEditorType::Count)> m_tools;
    AssetEditorType m_current_tool{ AssetEditorType ::None };
};

}  // namespace cave
