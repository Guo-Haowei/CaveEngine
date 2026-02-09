#pragma once
#include "editor/panels/EditorWindow.h"
#include "engine/private/renderer/graphics_defines.h"

namespace cave::render {
class CompiledGraph;
}  // namespace cave::render

namespace cave {

class RenderGraphViewer : public EditorWindow {
public:
    RenderGraphViewer(EditorState& p_editor);

    const char* GetWindowId() const override {
        return "Render Graph";
    }

protected:
    void DrawUIImpl() override;
    void DrawNodes(const render::CompiledGraph& p_graph);

    bool m_firstFrame{ true };
    Backend m_backend{ Backend::Null };
};

}  // namespace cave