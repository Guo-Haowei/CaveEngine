#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/view/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/Tab.h"

// @TODO: refactor
#include "editor/camera/CameraController.h"

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

    void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) override;
    void commitSceneChange() override {}
    void commitSceneReload() override;

    ViewId viewId() const override { return view_id_; }

protected:
    void submitView(bool support_pie);

    void updateRect(math::FloatRect& out_rect);

    void drawMainView(const math::FloatRect& rect);

    ViewManager& view_manager_;
    const ViewDimension dim_;
    SceneId preview_scene_id_;

    // @TODO: refactor
    std::unique_ptr<ICameraController> camera_controller_;
    CameraComponent camera_;
    TransformComponent camera_transform_;
    GpuTextureId texture_;
    ViewId view_id_;
};

}  // namespace cave
