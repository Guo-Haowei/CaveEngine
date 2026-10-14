#pragma once
#include "cave/render/rhi/Backend.h"

#include "editor/windows/EditorWindow.h"

namespace cave::render {
class CompiledGraph;
}  // namespace cave::render

namespace cave {

class RenderGraphViewer : public EditorWindow {
public:
    RenderGraphViewer(EditorState& p_editor);

    const char* windowId() const override {
        return "Render Graph";
    }

protected:
    void drawUIImpl() override;
    void DrawNodes(const render::CompiledGraph& p_graph);

    bool m_firstFrame{ true };
    render::Backend m_backend{ render::Backend::Null };
};

}  // namespace cave