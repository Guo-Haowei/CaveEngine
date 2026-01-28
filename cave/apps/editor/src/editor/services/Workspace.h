#pragma once
#include "engine/private/runtime/core/GenIdRegistry.h"

#include "editor/document/DocumentTypes.h"
#include "editor/windows/Tab.h"

// @TODO: deprecate
#include "editor/viewer/ViewerTabId.h"

namespace cave {

class EditorState;
class Guid;
class ViewerTab;

enum class SaveDialogResponse {
    Save,
    Discard,
    Cancel,
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

    static WorkspaceRequest OpenDoc(DocId p_doc_id) {
        WorkspaceRequest req{};
        req.type = Type::OpenDoc;
        req.doc_id = p_doc_id;
        return req;
    }
};

// @TODO: tab allocator
using TabId = GenId<Tab>;

class Workspace : public GenIdRegistry<Tab> {
public:
    Workspace(EditorState& p_editor);

    void Tick(float p_dt);

    void Submit(WorkspaceRequest p_req);
private:

    void OpenOrFocusDoc(DocId p_doc_id);

    bool RequestCloseDoc(DocId p_doc_id);

    bool RequestCloseTab(ViewerTabId p_tab_id);

    bool RequestCloseAll();

    DocId GetActiveDoc() const;
    ViewerTabId GetActiveTab() const;

    // Focus/activate
    bool FocusDoc(DocId doc_id);
    bool FocusTab(ViewerTabId tab_id);

    EditorState& m_editor;
    TabId m_focused_tab{};

    std::vector<WorkspaceRequest> m_pending_reqs;
    std::unordered_map<DocId, TabId> m_doc_to_tab;

    //----------------------------------------------------------------
    // @TODO: deprecate below apis

public:
    void RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close);

    // void SetCloseRequest(const TabId& p_id) { m_close_request = Some(p_id); }
    void HandleCloseRequest();
};

}  // namespace cave
