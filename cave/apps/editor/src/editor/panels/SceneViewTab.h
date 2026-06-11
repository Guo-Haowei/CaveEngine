#pragma once
#include "cave/runtime/view/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/ViewTabBase.h"
#include "editor/services/IPickConsumer.h"

// @TODO: refactor
#include "editor/Enums.h"

namespace cave {

class SceneViewTab : public ViewTabBase,
                     public IPickConsumer {
public:
    SceneViewTab(EditorState& editor,
                 DocId doc_id,
                 SceneId preview_scene_id,
                 ViewDimension dim);

    void onCreate() override;
    void onDestroy() override;

    Option<PickData> getPickData(const math::Vector2f& pos_screen) override;

    void onInputEvents(const InputFrame& input) override;

    DebugId debugId() const override { return debug_id_; }

protected:
    void submitView();

    void drawUIImpl() override;
    void drawGizmo(const math::FloatRect& rect);

    Scene* getResolvedScene();

    const DebugId debug_id_;

    GizmoAction gizmo_action_{ GizmoAction::Translate };

    std::array<const char*, 2> button_displays_;
    std::array<const char*, 2> button_tooltips_;

    ToolBarButtonDesc play_button_;

    int button_index_{ 0 };
};

}  // namespace cave
