#pragma once
#include "editor/windows/EditorWindow.h"

#include "cave/runtime/input/IInputConsumer.h"
#include "engine/private/runtime/framework/SceneView.h"
#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/document/DocumentTypes.h"

// @TODO: remove
#include "editor/widgets/ToolBar.h"

namespace cave {

enum ViewDimension : uint8_t {
    DIMENSION_2,
    DIMENSION_3,
};

class Tab : public EditorWindow,
            public IInputConsumer {
public:
    Tab(EditorState& p_editor,
        DocId p_doc_id,
        ViewDimension p_dim);

    const char* GetWindowId() const override { return m_window_id.c_str(); }

    void SetTitleAndId(std::string_view p_title, uint32_t p_idx);

    virtual void OnCreate();
    virtual void OnDestroy();

    int GetPriority() const override { return 10; }

    // @TODO: workspace decide which view the input should route to
    void OnEvents(const std::vector<InputEvent>&) override {}

    ViewDimension GetDimension() const { return m_dim; }

    DocId GetDocId() const { return m_doc_id; }

protected:
    void DrawUIImpl() override;

    // virtual const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const;

    const ViewDimension m_dim;
    DocId m_doc_id;
    std::string m_window_id;
    std::string m_title;
    uint32_t m_idx{ 0 };
};

}  // namespace cave
