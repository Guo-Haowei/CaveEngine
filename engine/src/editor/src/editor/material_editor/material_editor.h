#pragma once
#include "engine/scene/camera_component.h"

#include "editor/viewer/viewer_tab.h"

namespace cave {

using MaterialDocument = Document;

class MaterialEditor : public ViewerTab {
public:
    MaterialEditor(EditorLayer& p_editor, Viewer& p_viewer);

    bool HandleInput(const InputEvent* p_input_event) override;

    void OnCreate(const Guid& p_guid) override;

    void OnDestroy() override;

    void OnActivate() override;

    void DrawMainView(const CameraComponent& p_camera) override;

    void DrawAssetInspector() override;

    Document& GetDocument() const override;

    Scene* GetScene() override;

protected:
    const CameraComponent& GetActiveCameraInternal() const override;

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const override;

    std::shared_ptr<Scene> m_tmp_scene;
    std::shared_ptr<MaterialDocument> m_document;
    CameraComponent m_camera;
};

}  // namespace cave
