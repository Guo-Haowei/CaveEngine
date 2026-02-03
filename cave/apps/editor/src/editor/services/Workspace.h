#pragma once
#include "engine/private/core/GenIdRegistry.h"

#include "editor/document/DocId.h"
#include "editor/panels/Tab.h"

namespace cave {

class EditorState;
class Guid;

enum class SaveDialogResponse {
    Save,
    Discard,
    Cancel,
};

enum class DocKind : uint8_t {
    Scene,
    Script,
    Material,
    Mesh,
    Texture,
    Audio,
    Prefab,
    Shader,

    _Count,
};

struct WorkspaceRequest {
    enum class Type {
        OpenDoc,
        OpenPath,
        NewDoc,
        CloseDoc,
        CloseAll,
        FocusDoc,
    } type;

    DocKind kind{};
    DocId doc_id{};
    std::string path;

    static WorkspaceRequest Open(DocId p_doc_id) {
        WorkspaceRequest req{};
        req.type = Type::OpenDoc;
        req.doc_id = p_doc_id;
        return req;
    }

    static WorkspaceRequest Close(DocId p_doc_id) {
        WorkspaceRequest req{};
        req.type = Type::CloseDoc;
        req.doc_id = p_doc_id;
        return req;
    }
};

class Workspace final : protected GenIdRegistry<Tab>,
                        public IInputConsumer {
public:
    Workspace(EditorState& p_editor);
    ~Workspace();

    void Tick(float p_dt);

    void Submit(WorkspaceRequest p_req);

    TabId GetFocusedTabId() const { return m_focused_tab; }

    Tab* GetFocusedTab() { return Resolve(m_focused_tab); }

    void OnEvents(const std::vector<InputEvent>& p_events) final;

    int GetPriority() const final { return 10; }

    DebugId GetDebugId() final { return m_debug_id; }

    bool OnCloseRequested();

private:
    void OpenOrFocusDoc(DocId p_doc_id);

    bool CloseDoc(DocId p_doc_id);

    bool RequestCloseAll();

    DocId GetActiveDoc() const;

    // Focus/activate
    bool FocusDoc(DocId doc_id);

    void FlushPendingRequests();
    void DrawTabs();

    EditorState& m_editor;
    const DebugId m_debug_id;

    TabId m_focused_tab{};
    TabId m_focused_req{};

    std::vector<WorkspaceRequest> m_pending_reqs;
    std::unordered_map<DocId, TabId> m_doc_to_tab;
};

}  // namespace cave
