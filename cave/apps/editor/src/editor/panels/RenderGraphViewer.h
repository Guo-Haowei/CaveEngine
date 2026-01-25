#pragma once
#include "editor/EditorWindow.h"
#include "engine/private/renderer/graphics_defines.h"

namespace cave {
class RenderGraph;
}

namespace cave {

class RenderGraphViewer : public EditorWindow {
public:
    RenderGraphViewer(EditorState& p_editor);

    const char* GetTitle() const override {
        return "Render Graph";
    }

protected:
    void UpdateInternal(float p_timestep) override;
    void DrawNodes(const RenderGraph& p_graph);

    bool m_firstFrame{ true };
    Backend m_backend{ Backend::COUNT };
};

}  // namespace cave