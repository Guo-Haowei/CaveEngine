#pragma once
#include "cave/runtime/intent/IIntentHandler.h"

#include "engine/private/core/ids/GenIdRegistry.h"

#include "editor/document/DocId.h"
#include "editor/panels/Tab.h"

namespace cave {

class EditorState;
class Guid;

struct PreviewScene {
    DocId doc_id{};
    ViewId view_id{};
    SceneId scene_id{};
    Scene* scene{ nullptr };
};

class Workspace final : protected GenIdRegistry<Tab>,
                        public IInputConsumer,
                        public IIntentHandler {
public:
    Workspace(EditorState& p_editor);
    ~Workspace();

    void Tick();

    void RequestOpen(DocId p_doc_id);
    void RequestClose(DocId p_doc_id);

    TabId FocusedTabId() const { return m_focused_tab; }

    Tab* FocusedTab() { return Resolve(m_focused_tab); }

    DocId FocusedDoc();

    PreviewScene FocusedPreviewScene();

    bool HandleIntent(Intent& p_intent) override;
    void OnEvents(const InputFrame& p_input) override;

    int GetPriority() const override { return 10; }

    DebugId GetDebugId() const override { return m_debug_id; }

    bool OnCloseRequested();

private:
    void OpenOrFocusDoc(DocId p_doc_id);

    bool CloseDoc(DocId p_doc_id);

    bool RequestCloseAll();

    DocId GetActiveDoc() const;

    // Focus/activate
    bool FocusDoc(DocId doc_id);

    void DrawTabs();

    EditorState& m_editor;

    TabId m_focused_tab{};
    TabId m_focused_req{};

    std::unordered_map<DocId, TabId> m_doc_to_tab;
    const DebugId m_debug_id;
};

}  // namespace cave
