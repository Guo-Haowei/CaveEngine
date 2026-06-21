#pragma once
#include "editor/panels/EditorWindow.h"
#include "editor/widgets/SpriteSelector.h"

namespace cave {

struct EditorServices;

class IDocument;

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorState& editor,
                   EditorServices& editor_services);

    const char* windowId() const override {
        return "Asset Inspector";
    }

    void onAttach() override;

protected:
    void drawUIImpl() override;

    EditorServices& editor_services_;
};

}  // namespace cave
