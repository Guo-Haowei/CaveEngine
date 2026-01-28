#pragma once
#include "engine/private/runtime/scene/CameraComponent.h"

namespace cave {

#if 0
struct MaterialAsset;

class MaterialEditor : public ViewerTab {
public:
    MaterialEditor(EditorState& p_editor, Viewer& p_viewer);

    void OnDestroy() final;

    void DrawMainView(const CameraComponent& p_camera) final;

    void DrawAssetInspector() final;

protected:
    void OnCreateInternal(const Guid& p_guid) final;

    void OnActivateInternal() final;

    void DrawTextureSlots(MaterialAsset& p_material);

    const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    std::shared_ptr<Scene> m_tmp_scene;
    std::shared_ptr<MaterialDocument> m_document;
};
#endif

}  // namespace cave
