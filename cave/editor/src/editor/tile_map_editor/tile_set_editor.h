#pragma once
#include "engine/assets/asset_handle.h"

#include "editor/viewer/viewer_tab.h"
#include "editor/widgets/sprite_selector.h"

namespace cave {

class Document;

class TileSetEditor : public ViewerTab {
public:
    TileSetEditor(EditorLayer& p_editor, Viewer& p_viewer);
    ~TileSetEditor();

    bool HandleInput(const InputEvent* p_input_event) override;

    void OnCreate(const Guid& p_guid) override;

    void OnDestroy() override;

    void OnActivate() override;

    void DrawMainView(const CameraComponent& p_camera) override;

    void DrawAssetInspector() override;

    Document& GetDocument() const override;

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const override;

    std::unique_ptr<CameraComponent> m_camera;
    std::unique_ptr<Document> m_document;

    ToolBarButtonDesc m_desc;

    SpriteSelector m_sprite_selector;
};

}  // namespace cave
