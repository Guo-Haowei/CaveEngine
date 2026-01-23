#pragma once
#include "viewer_tab_manager.h"

#include "engine/input/input_router.h"
#include "engine/scene/camera_controller.h"

#include "editor/editor_window.h"
#include "editor/enums.h"

namespace cave {

class Viewer : public EditorWindow {
public:
    Viewer(EditorState& p_editor);

    Option<Vector2f> CursorToNDC(Vector2f p_point) const;

    const Vector2f& GetCanvasMin() const { return m_canvas_min; }
    const Vector2f& GetCanvasSize() const { return m_canvas_size; }

    void OpenTab(AssetType p_type, const Guid& p_guid);
    ViewerTab* GetActiveTab();

    const char* GetTitle() const override {
        return "Viewer";
    }

protected:
    void UpdateInternal(float p_timestep) override;

    void UpdateFrameSize();

    Vector2f m_canvas_min;
    Vector2f m_canvas_size;

    ViewerTabManager m_tab_manager;
};

}  // namespace cave
