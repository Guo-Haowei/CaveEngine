#pragma once
#include "cave/rhi/Backend.h"

#include "editor/panels/EditorWindow.h"

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
    rhi::Backend m_backend{ rhi::Backend::Null };
};

}  // namespace cave