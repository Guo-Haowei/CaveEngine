#pragma once
#include "viewer_tab_manager.h"

#include "engine/input/input_router.h"
#include "engine/scene/camera_controller.h"
#include "editor/editor_window.h"
#include "editor/enums.h"

namespace cave {

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

class Viewer : public EditorWindow {
public:
    Viewer(EditorLayer& p_editor);

    bool HandleInput(const InputEvent* p_input_event);

    Option<Vector2f> CursorToNDC(Vector2f p_point) const;

    const Vector2f& GetCanvasMin() const { return m_canvas_min; }
    const Vector2f& GetCanvasSize() const { return m_canvas_size; }

    InputCache& GetInputCache() { return m_camera_input; }
    const InputCache& GetInputCache() const { return m_camera_input; }

    void OpenTab(AssetType p_type, const Guid& p_guid);
    ViewerTab* GetActiveTab();

    const char* GetTitle() const override {
        return "Viewer";
    }

protected:
    void UpdateInternal(Scene* p_scene) override;

    void DrawToolBar();

    void UpdateFrameSize();
    bool CacheCameraInput(const InputEvent* p_input_event);

    Vector2f m_canvas_min;
    Vector2f m_canvas_size;

    ViewerTabManager m_tab_manager;
    InputCache m_camera_input;

    CameraControllerFPS m_controller_3d;
    CameraController2DEditor m_controller_2d;
};

}  // namespace cave
