#pragma once
#include "engine/assets/asset_handle.h"

#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/sprite_selector.h"

namespace cave {

class Document;

class TileSetEditor : public ViewerTab {
public:
    TileSetEditor(EditorState& p_editor, Viewer& p_viewer);
    ~TileSetEditor();

    bool HandleInput(const OldInputEvent* p_input_event) final;

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    void DrawAssetInspector() final;

    Document& GetDocument() const final;

    Scene* GetScene() final {
        return nullptr;
    }

protected:
    void OnCreateInternal(const Guid& p_guid) final;

    void OnActivateInternal() final;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    void DrawPhysicsTab(TileSetAsset& p_tile_set);

    std::unique_ptr<Document> m_document;

    SpriteSelector m_sprite_selector;
};

}  // namespace cave
