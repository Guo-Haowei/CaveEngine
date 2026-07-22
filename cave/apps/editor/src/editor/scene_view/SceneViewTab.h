#pragma once
#include "cave/runtime/view/ViewDesc.h"

#include "editor/document/SceneDocument.h"
#include "editor/windows/ViewTabBase.h"
#include "editor/scene_view/ISceneViewTool.h"

namespace cave {

class TilePaintTool;

class SceneViewTab : public ViewTabBase,
                     public IPickConsumer {
public:
    SceneViewTab(EditorState& editor,
                 DocId doc_id,
                 SceneId preview_scene_id,
                 ViewDimension dim);

    Option<PickData> getPickData(const math::Vec2f& pos_screen) override;

    DebugId debugId() const override { return m_debug_id; }

private:
    void onCreate() override;
    void onDestroy() override;

    void onInputEvents(const InputFrame& input) override;

    void drawAssetInspector(IDocument& doc) override;

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

    TilePaintTool* m_tile_paint_tool = nullptr;
    std::array<Owner<ISceneViewTool>, std::to_underlying(SceneViewToolType::Count)> m_scene_tools;
    SceneViewToolType m_current_tool = SceneViewToolType::Select;
};

}  // namespace cave
