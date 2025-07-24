#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/assets/asset_interface.h"
#include "editor/editor_window.h"

namespace cave {

class AssetRegistry;

class AssetInspector : public EditorWindow {
public:
    AssetInspector(EditorLayer& p_editor);

    void OnAttach() override;

    const char* GetTitle() const override {
        return "Asset Inspector";
    }

protected:
    void UpdateInternal() override;
    void DrawContentBrowser();
    void DrawAssets();
};

}  // namespace cave
