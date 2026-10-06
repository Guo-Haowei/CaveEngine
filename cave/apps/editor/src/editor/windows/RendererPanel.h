#pragma once
#include "editor/windows/EditorWindow.h"

// clang-format off
namespace cave::render { class Renderer; }
// clang-format on

namespace cave {

class RendererPanel : public EditorWindow {
public:
    RendererPanel(EditorState& editor);

    const char* windowId() const override {
        return "Renderer";
    }

private:
    void drawUIImpl() override;

    render::Renderer& m_renderer;
};

}  // namespace cave
