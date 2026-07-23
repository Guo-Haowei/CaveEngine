#pragma once
#include <span>

#include "cave/core/ids/GenIdRegistry.h"
#include "cave/core/ids/Guid.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/intent/IIntentHandler.h"

#include "editor/document/DocId.h"
#include "editor/services/WorkspaceState.h"

namespace cave {

struct EngineServices;
struct EditorServices;

class EditorState;
class Scene;

struct PreviewScene {
    DocId doc_id{};
    ViewId view_id{};
    Guid guid;
    SceneId scene_id{};
    Scene* scene{ nullptr };
};

class Workspace final : protected GenIdRegistry<Tab>,
                        public IInputConsumer,
                        public IIntentHandler {
public:
    Workspace(EditorState& editor);
    ~Workspace();

    void tick(float dt);

    void requestOpen(DocId doc_id);
    void requestClose(DocId doc_id);

    TabId focusedTabId() const { return m_focused_tab; }
    Tab* focusedTab() { return resolve(m_focused_tab); }

    DocId focusedDoc();

    PreviewScene focusedPreviewScene();

    bool handleIntent(Intent& intent) override;

    void onEvents(const InputFrame& input) override;
    int priority() const override { return 10; }
    DebugId debugId() const override { return m_debug_id; }

    void onAssetChanged(const Guid& changed, std::span<const Guid> affected);

    WorkspaceState& workspaceState() { return m_workspace_state; }
    const WorkspaceState& workspaceState() const { return m_workspace_state; }

    void restoreTabs();

private:
    void openOrFocusDoc(DocId doc_id);
    void drawTabs();
    bool closeDoc(DocId doc_id);

    void refreshTabStates();

    void saveWorkspaceState(float dt);
    bool loadWorkspaceState();
    bool buildStateCachePath();

    EditorState& m_editor;
    EngineServices& m_engine_services;
    EditorServices& m_editor_services;
    const DebugId m_debug_id;

    TabId m_focused_tab{};
    TabId m_request_focus{};

    std::unordered_map<DocId, TabId> m_doc_to_tab;
    std::unordered_map<Guid, TabId> m_guid_to_tab;

    WorkspaceState m_workspace_state;
    std::filesystem::path m_workspace_file;
};

}  // namespace cave
