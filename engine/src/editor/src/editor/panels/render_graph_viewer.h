#pragma once
#include "editor/editor_window.h"
#include "engine/core/base/graph.h"
#include "engine/renderer/graphics_defines.h"

namespace cave {
class RenderGraph;
}

namespace cave {

class RenderGraphViewer : public EditorWindow {
public:
    RenderGraphViewer(EditorLayer& p_editor);

    const char* GetTitle() const override {
        return "Render Graph";
    }

protected:
    void UpdateInternal() override;
    void DrawNodes(const RenderGraph& p_graph);

    bool m_firstFrame{ true };
    Backend m_backend{ Backend::COUNT };
};

}  // namespace cave