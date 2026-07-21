#pragma once
#include "cave/runtime/view/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/panels/ViewTabBase.h"
#include "editor/scene_view/ISceneViewTool.h"

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

    Option<PickData> getPickData(const math::Vec2f& pos_screen) override;

    void onInputEvents(const InputFrame& input) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    void submitView();

    void drawUIImpl() override;

    void drawToolbar() override;

    bool onAssetDropped(AssetHandle handle) override;

    Scene* getResolvedScene();

    ISceneViewTool* activeTool();

    EditorState& m_editor;
    const DebugId m_debug_id;

    ToolbarButtonDesc m_play_button;
    ToolbarButtonDesc m_pause_button;

    std::array<Owner<ISceneViewTool>, std::to_underlying(SceneViewToolType::Count)> m_scene_tools;
    SceneViewToolType m_current_tool = SceneViewToolType::Select;
};

}  // namespace cave
