#pragma once
#include "editor/document/SceneDocument.h"
#include "editor/windows/Tab.h"

#include "../Enums.h"

namespace cave {

class SceneEditor : public Tab,
                    public ISceneTickContributor {
public:
    SceneEditor(EditorState& p_editor,
                DocId p_doc_id,
                SceneId p_scene_id,
                ViewDimension p_dim);

    void OnCreate() final;
    void OnDestroy() final;

    void CollectSceneTicks(std::vector<SceneTickRequest>& p_out) override;

protected:
    //void OnCreateInternal(const Guid& p_guid) final;

    //void OnActivateInternal() final;

    //const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const final;

    GizmoAction m_state{ GizmoAction::Translate };

    void Select(const Vector2f& p_cursor);

    std::array<const char*, 2> m_button_displays;
    std::array<const char*, 2> m_button_tooltips;

    int m_button_index{ 0 };

    ToolBarButtonDesc m_play_button;
};

}  // namespace cave
