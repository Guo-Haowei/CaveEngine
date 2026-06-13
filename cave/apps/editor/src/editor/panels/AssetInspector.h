#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

struct EditorServices;

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorState& editor,
                   EditorServices& editor_services);

    const char* windowId() const override {
        return "Asset Inspector";
    }

protected:
    void drawUIImpl() override;

    EditorServices& editor_services_;
};

}  // namespace cave
