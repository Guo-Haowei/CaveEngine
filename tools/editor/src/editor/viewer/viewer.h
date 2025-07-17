#pragma once
#include "tab_id.h"

#include "engine/input/input_router.h"
#include "engine/scene/camera_controller.h"
#include "editor/editor_window.h"
#include "editor/enums.h"

namespace cave {

class TabId;
class ViewerTab;

class TabManager {
public:
    void SwitchTab(const TabId& p_id);
    void SwitchTab(std::shared_ptr<ViewerTab>&& p_tab);

    Option<ViewerTab*> FindTabById(const TabId& p_id);
    Option<ViewerTab*> FindTabByGuid(const Guid& p_guid);
    Option<ViewerTab*> GetActiveTab();

    void RequestClose(const TabId& p_id) { m_close_request = p_id; }
    void HandleTabClose();

    enum class SaveDialogResponse {
        Save,
        Discard,
        Cancel,
    };

    void RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close);

    const Option<TabId>& GetFocusRequest() const { return m_focus_request; }

    auto& GetTabs() { return m_tabs; }

private:
    Option<TabId> m_focus_request;
    Option<TabId> m_close_request;

    Option<TabId> m_active_tab;
    std::unordered_map<TabId, std::shared_ptr<ViewerTab>> m_tabs;
};

class Viewer : public EditorWindow, public IInputHandler {
public:
    Viewer(EditorLayer& p_editor);

    HandleInputResult HandleInput(std::shared_ptr<InputEvent> p_input_event) override;

    std::optional<Vector2f> CursorToNDC(Vector2f p_point) const;

    const Vector2f& GetCanvasMin() const { return m_canvas_min; }
    const Vector2f& GetCanvasSize() const { return m_canvas_size; }

    CameraComponent& GetActiveCamera() { return m_controller.cameras[m_controller.current]; }

    auto& GetInputState() { return m_input_state; }

    void OpenTab(AssetType p_type, const Guid& p_guid);
    ViewerTab* GetActiveTab();

    const char* GetTitle() const override {
        return "Viewer";
    }

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

    // @TODO: refactor
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

    TabManager m_tab_manager;
};

}  // namespace cave
