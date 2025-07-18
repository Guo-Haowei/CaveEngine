#pragma once
#include "viewer_tab_manager.h"

#include "engine/input/input_router.h"
#include "editor/editor_window.h"
#include "editor/enums.h"

namespace cave {

struct OldInputState {
    int dx, dy, dz;
    float scroll;
    Vector2f mouse_move;
    std::optional<Vector2f> ndc;
    std::array<bool, 3> buttons;

    OldInputState() { Reset(); }

    void Reset() {
        dx = dy = dz = 0;
        scroll = 0.0f;
        mouse_move = Vector2f{ 0, 0 };
        ndc = std::nullopt;
        buttons.fill(0);
    }
};

class Viewer : public EditorWindow, public IInputHandler {
public:
    Viewer(EditorLayer& p_editor);

    HandleInputResult HandleInput(std::shared_ptr<InputEvent> p_input_event) override;

    std::optional<Vector2f> CursorToNDC(Vector2f p_point) const;

    const Vector2f& GetCanvasMin() const { return m_canvas_min; }
    const Vector2f& GetCanvasSize() const { return m_canvas_size; }

    auto& GetInputState() { return m_input_state; }
    const auto& GetInputState() const { return m_input_state; }

    void OpenTab(AssetType p_type, const Guid& p_guid);
    ViewerTab* GetActiveTab();

    const char* GetTitle() const override {
        return "Viewer";
    }

protected:
    void UpdateInternal(Scene* p_scene) override;

    void DrawToolBar();

    void UpdateFrameSize();
    HandleInputResult HandleInputCamera(std::shared_ptr<InputEvent> p_input_event);

    Vector2f m_canvas_min;
    Vector2f m_canvas_size;
    bool m_focused;

    ViewerTabManager m_tab_manager;
    OldInputState m_input_state;
};

}  // namespace cave
