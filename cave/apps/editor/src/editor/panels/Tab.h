#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/input/IInputConsumer.h"

#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/document/DocId.h"
#include "editor/panels/EditorWindow.h"

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
    Tab(EditorState& editor, DocId doc_id);

    void setTitleAndId(std::string_view title, uint32_t idx);

    void drawUI() override;
    virtual void drawAssetInspector(IDocument&) {}

    const char* windowId() const override { return window_id_.c_str(); }

    virtual void onCreate();
    virtual void onDestroy();

    virtual void onInputEvents(const InputFrame&) {}

    DocId docId() const { return doc_id_; }
    virtual ViewId viewId() const { return ViewId{}; }

    TabId tabId() const { return tab_id_; }
    void tabId(TabId tab_id) { tab_id_ = tab_id; }

protected:
    void drawUIImpl() override {}

    // virtual const std::vector<const ToolBarButtonDesc*> GetToolBarButtons() const;

    DocId doc_id_;
    TabId tab_id_;
    uint32_t idx_{ 0 };
    std::string window_id_;
    std::string title_;
};

}  // namespace cave
