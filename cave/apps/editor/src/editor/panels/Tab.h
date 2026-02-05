#pragma once
#include "editor/panels/EditorWindow.h"

#include "cave/runtime/input/IInputConsumer.h"
#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/document/DocId.h"

// @TODO: remove
#include "editor/widgets/ToolBar.h"

namespace cave {

class Tab;

// @TODO: move it to somewhere else
using TabId = GenId<Tab>;

enum class CloseDecision {
    Save,
    Discard,
    Cancel,
};

class Tab : public EditorWindow {
public:
    Tab(EditorState& p_editor, DocId p_doc_id);

    const char* GetWindowId() const override { return m_window_id.c_str(); }

    void SetTitleAndId(std::string_view p_title, uint32_t p_idx);

    virtual void OnCreate();
    virtual void OnDestroy();

    void DrawUI() override;

    virtual void OnInputEvents(const InputFrame&) {}

    DocId GetDocId() const { return m_doc_id; }

    TabId GetTabId() const { return m_tab_id; }
    void SetTabId(TabId p_tab_id) { m_tab_id = p_tab_id; }

protected:
    void DrawUIImpl() override {}

    // virtual const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const;

    DocId m_doc_id;
    TabId m_tab_id;
    uint32_t m_idx{ 0 };
    std::string m_window_id;
    std::string m_title;
};

}  // namespace cave
