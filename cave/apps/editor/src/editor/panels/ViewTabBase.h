#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/view/ViewDesc.h"

#include "editor/camera/CameraController.h"
#include "editor/document/SceneDocument.h"
#include "editor/panels/Tab.h"

// @TODO: refactor
#include "engine/private/runtime/scene/SceneOwner.h"

namespace cave {

class ViewManager;

enum class ViewDimension : uint8_t {
    Dim2,
    Dim3,
};

class ViewTabBase : public Tab,
                    public SceneOwner {
public:
    ViewTabBase(EditorState& editor,
                DocId doc_id,
                SceneId scene_id,
                ViewDimension dim);

    void onCreate() override;
    void onDestroy() override;

    void collectSceneTicks(Vector<SceneTickRequest>& out_requests) override;
    virtual bool onAssetDropped(AssetHandle handle);

    ViewId viewId() const override { return m_view_id; }

private:
    void commitSceneChange(String&&) override {}
    void commitSceneReload() override;

    void drawMainViewImpl(const math::FloatRect& rect);
    virtual void drawToolbar() {}

protected:
    void submitView(bool support_pie);
    void drawMainView(const math::FloatRect& rect);
    void updateRect(math::FloatRect& out_rect);

    bool tabState(TabState& out) const override;

    EditorState& m_editor;
    ViewManager& m_view_manager;
    const ViewDimension m_dim;
    SceneId m_preview_scene_id;

    // @TODO: refactor
    Owner<ICameraController> m_camera_controller;
    CameraComponent m_camera;
    TransformComponent m_camera_transform;
    GpuTextureId m_texture;
    ViewId m_view_id;
};

}  // namespace cave
