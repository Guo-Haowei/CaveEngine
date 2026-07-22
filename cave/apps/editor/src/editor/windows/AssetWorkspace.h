#pragma once
#include "editor/windows/EditorWindow.h"
#include "editor/widgets/AtlasWidget.h"

namespace cave {

struct EditorServices;

class IDocument;

class AssetWorkspace : public EditorWindow {
public:
    explicit AssetWorkspace(EditorState& editor);

    const char* windowId() const override {
        return "Asset Workspace";
    }

    void onAttach() override;

protected:
    void drawUIImpl() override;

    ImageCanvas m_image_canvas;
};

}  // namespace cave
