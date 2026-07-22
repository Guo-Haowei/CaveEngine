#pragma once
#include "editor/panels/EditorWindow.h"
#include "editor/widgets/AtlasWidget.h"

namespace cave {

struct EditorServices;

class IDocument;

class AssetInspector : public EditorWindow {
public:
    explicit AssetInspector(EditorState& editor);

    const char* windowId() const override {
        return "Asset Inspector";
    }

    void onAttach() override;

protected:
    void drawUIImpl() override;

    ImageCanvas m_image_canvas;
};

}  // namespace cave
