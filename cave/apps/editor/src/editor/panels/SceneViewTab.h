#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Rect.h"
#include "cave/runtime/view/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/Tab.h"
#include "editor/services/IPickConsumer.h"

// @TODO: refactor
#include "editor/Enums.h"
#include "editor/camera/CameraController.h"

namespace cave {

class ViewManager;

enum class ViewDimension : uint8_t {
    Dim2,
    Dim3,
};

class SceneViewTab : public Tab,
                     public IPickConsumer,
                     public ISceneTickContributor {
public:
    SceneViewTab(EditorState& editor,
                 DocId doc_id,
                 SceneId preview_scene_id,
                 ViewDimension dim);

    void onCreate() override;
    void onDestroy() override;

    void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) override;

    Option<PickData> GetPickData(const math::Vector2f& pos_screen) override;

    void onInputEvents(const InputFrame& input) override;

    ViewId viewId() const override { return view_id_; }

    DebugId debugId() const final { return debug_id_; }

protected:
    void submitView();

    void drawUIImpl() override;

    void updateRect(math::FloatRect& out_rect);
    void drawMainView(const math::FloatRect& rect);
    void drawGizmo(const math::FloatRect& rect);

    Scene* getResolvedScene();
    // void OnCreateInternal(const Guid& p_guid) final;

    // void OnActivateInternal() final;

    // const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    ViewManager& view_manager_;
    const ViewDimension dim_;
    const DebugId debug_id_;
    GizmoAction gizmo_action_{ GizmoAction::Translate };

    SceneId preview_scene_id_;

    std::array<const char*, 2> button_displays_;
    std::array<const char*, 2> button_tooltips_;

    ToolBarButtonDesc play_button_;

    int button_index_{ 0 };

    // @TODO: move to input controller
    std::unique_ptr<ICameraController> camera_controller_;
    CameraComponent camera_;
    TransformComponent camera_transform_;
    GpuTextureId texture_;
    ViewId view_id_;
};

}  // namespace cave
