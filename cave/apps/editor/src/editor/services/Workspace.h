#pragma once
#include "cave/runtime/intent/IIntentHandler.h"

#include "engine/private/core/ids/GenIdRegistry.h"

#include "editor/document/DocId.h"
#include "editor/panels/Tab.h"

namespace cave {

struct EngineServices;
struct EditorServices;

class EditorState;
class Guid;

struct PreviewScene {
    DocId doc_id{};
    ViewId view_id{};
    SceneId scene_id{};
    Scene* scene{ nullptr };
};

struct ContentBrowserState {
    std::string current_path = "@res://";
};

struct WorkspaceState {
    ContentBrowserState content_browser;

    std::vector<TabState> tabs;
};

class Workspace final : protected GenIdRegistry<Tab>,
                        public IInputConsumer,
                        public IIntentHandler {
public:
    Workspace(EditorState& editor);
    ~Workspace();

    void tick();

    void requestOpen(DocId doc_id);
    void requestClose(DocId doc_id);

    TabId focusedTabId() const { return focused_tab_; }

    Tab* focusedTab() { return resolve(focused_tab_); }

    DocId focusedDoc();

    PreviewScene focusedPreviewScene();

    bool handleIntent(Intent& intent) override;

    bool onCloseRequested();

    void onEvents(const InputFrame& input) override;
    int priority() const override { return 10; }
    DebugId debugId() const override { return debug_id_; }

    void onAssetChanged(const Guid& changed, std::span<const Guid> affected);

    WorkspaceState& workspaceState() { return workspace_state_; }
    const WorkspaceState& workspaceState() const { return workspace_state_; }

private:
    void openOrFocusDoc(DocId doc_id);
    void drawTabs();
    bool closeDoc(DocId doc_id);

    void refreshTabStates();
    bool loadWorkspaceState(std::string_view path);
    void saveWorkspaceState();

    EditorState& editor_;
    EngineServices& app_services_;
    EditorServices& editor_services_;
    const DebugId debug_id_;

    TabId focused_tab_{};
    TabId request_focus_{};

    std::unordered_map<DocId, TabId> doc_to_tab_;
    std::unordered_map<Guid, TabId> guid_to_tab_;

    WorkspaceState workspace_state_;
};

}  // namespace cave
