#pragma once
#include "editor/panels/EditorWindow.h"

namespace cave {

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorState& editor);

    const char* windowId() const override {
        return "Asset Inspector";
    }

protected:
    void drawUIImpl() override;
};

}  // namespace cave
