#pragma once
#include "engine/scene/camera_component.h"

#include "editor/viewer/viewer_tab.h"

namespace cave {

struct MaterialAsset;

using MaterialDocument = Document;

class MaterialEditor : public ViewerTab {
public:
    MaterialEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) override;

    void OnDestroy() override;

    void DrawMainView(const CameraComponent& p_camera) override;

    void DrawAssetInspector() override;

    Document& GetDocument() const override;

    Scene* GetScene() override;

protected:
    void OnCreateInternal(const Guid& p_guid) override;

    void OnActivateInternal() override;

    void DrawTextureSlots(MaterialAsset& p_material);

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const override;

    std::shared_ptr<Scene> m_tmp_scene;
    std::shared_ptr<MaterialDocument> m_document;
};

}  // namespace cave
