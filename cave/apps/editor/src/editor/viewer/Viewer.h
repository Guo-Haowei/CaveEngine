#pragma once

// @TODO: refactor
#include "editor/panels/EditorWindow.h"

namespace cave {

#if 0
class Workspace;
class ViewerTab;

class Viewer : public EditorWindow {
public:
    Viewer(EditorState& p_editor);

    Option<Vector2f> CursorToNDC(Vector2f p_point) const;

    const Vector2f& GetCanvasMin() const { return m_canvas_min; }
    const Vector2f& GetCanvasSize() const { return m_canvas_size; }

    // @TODO: deprecate
    ViewerTab* GetActiveTab();

    const char* GetWindowId() const override {
        return "Viewer###(MainWorkSpace)";
    }

protected:
    void UpdateInternal(float p_timestep) override;

    void UpdateFrameSize();

    Workspace& m_workspace;

    Vector2f m_canvas_min;
    Vector2f m_canvas_size;
};
#endif

}  // namespace cave
