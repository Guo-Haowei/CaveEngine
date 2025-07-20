#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/sprite/sprite_animation_asset.h"
#include "editor/viewer/viewer_tab.h"

namespace cave {

class AssetRegistry;
class Document;
class InputEvent;
class TileMapAsset;
class TileMapDocument;
class Viewer;

using SpriteAnimationDocument = Document;

class SpriteAnimationEditor : public ViewerTab {
public:
    SpriteAnimationEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) override;

    void OnCreate(const Guid& p_guid) override;

    void OnDestroy() override;

    void OnActivate() override;

    void DrawMainView(const CameraComponent& p_camera) override;

    Document& GetDocument() const override;

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    AssetRegistry* m_asset_registry = nullptr;

    std::shared_ptr<Scene> m_tmp_scene;

    std::unique_ptr<CameraComponent> m_camera;

    std::shared_ptr<SpriteAnimationDocument> m_document;
};

}  // namespace cave
