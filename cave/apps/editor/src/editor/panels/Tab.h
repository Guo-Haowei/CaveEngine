#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/runtime/ecs/components/CameraComponent.h"
#include "cave/runtime/ecs/components/TransformComponent.h"
#include "cave/runtime/input/IInputConsumer.h"

#include "engine/private/runtime/scene/SceneScheduler.h"

#include "editor/document/DocId.h"
#include "editor/panels/EditorWindow.h"

// @TODO: remove
#include "editor/widgets/ToolBar.h"

namespace cave {

class Tab;

using TabId = GenId<Tab>;

enum class CloseDecision {
    Save,
    Discard,
    Cancel,
};

struct TabState {
    Guid guid;
    Option<CameraComponent> camera;
    Option<TransformComponent> transform;
};

class Tab : public EditorWindow {
public:
    Tab(EditorState& editor, DocId doc_id);

    void setTitleAndId(std::string_view title, uint32_t idx);

    void drawUI() override;
    virtual void drawAssetInspector(IDocument&) {}

    const char* windowId() const override;

    virtual void onCreate();
    virtual void onDestroy();

    virtual void onInputEvents(const InputFrame&) {}

    virtual bool tabState(TabState& out) const;

    DocId docId() const { return doc_id_; }
    virtual ViewId viewId() const { return ViewId{}; }

    TabId tabId() const { return tab_id_; }
    void tabId(TabId tab_id) { tab_id_ = tab_id; }

protected:
    void drawUIImpl() override {}

    DocId doc_id_;
    TabId tab_id_;
    mutable std::string window_id_;
};

}  // namespace cave
