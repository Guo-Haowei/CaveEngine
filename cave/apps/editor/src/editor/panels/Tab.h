#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/ids/Guid.h"
#include "cave/runtime/ecs/components/CameraComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/input/IInputConsumer.h"

#include "editor/document/DocId.h"
#include "editor/panels/EditorWindow.h"

// @TODO: remove
#include "editor/widgets/ToolBar.h"
// #include "engine/private/runtime/scene/SceneScheduler.h"

namespace cave {

class Tab;

using TabId = GenId<Tab>;

enum class CloseDecision {
    Save,
    Discard,
    Cancel,
};

struct TabState {
    bool active;
    Guid guid;
    Option<CameraComponent> camera;
    Option<TransformComponent> transform;
};

class Tab : public EditorWindow {
public:
    Tab(EditorState& editor, DocId doc_id);

    void drawUI() override;
    virtual void drawAssetInspector(IDocument&) {}

    const char* windowId() const override;

    virtual void onCreate();
    virtual void onDestroy();

    virtual void onInputEvents(const InputFrame&) {}

    virtual bool tabState(TabState& out) const;

    DocId docId() const { return m_doc_id; }
    virtual ViewId viewId() const { return ViewId{}; }

    TabId tabId() const { return m_tab_id; }
    void tabId(TabId tab_id) { m_tab_id = tab_id; }

protected:
    void drawUIImpl() override {}

    DocId m_doc_id;
    TabId m_tab_id;
    mutable std::string m_window_id;
};

}  // namespace cave
