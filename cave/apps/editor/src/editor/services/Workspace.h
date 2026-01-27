#pragma once
#include "editor/document/DocumentTypes.h"

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

class Workspace {
public:
    Workspace(EditorState& p_editor);

    void SendRequest(WorkspaceRequest p_request);

    void Tick(float p_dt);

private:
    void OpenOrFocusDoc(DocId p_doc_id);

    bool RequestCloseDoc(DocId p_doc_id);

    bool RequestCloseTab(TabId p_tab_id);

    bool RequestCloseAll();

    DocId GetActiveDoc() const;
    TabId GetActiveTab() const;

    // Focus/activate
    bool FocusDoc(DocId doc_id);
    bool FocusTab(TabId tab_id);

    // Navigation / UI menus
    // void ListOpenTabs(std::vector<TabInfo>& out) const;
    // void ListOpenDocs(std::vector<DocInfo>& out) const;

    // void RequestOpenFileDialog(DocKind kind);

    //// Direct open by path (menu recent files, drag/drop).
    // void RequestOpenPath(DocKind kind, std::string_view path);

    //// New doc (untitled) + tab.
    // void RequestNewDoc(DocKind kind);

    EditorState& m_editor;

    // @TODO: change to unique_ptr
    std::unordered_map<DocId, std::shared_ptr<ViewerTab>> m_tabs;
    std::vector<WorkspaceRequest> m_pending_reqs;

    //----------------------------------------------------------------
    // @TODO: deprecate below apis

    ViewerTab* m_active_tab = nullptr;

public:
    ViewerTab* GetActiveTab() { return m_active_tab; }

    void RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close);

    // void SetCloseRequest(const TabId& p_id) { m_close_request = Some(p_id); }
    void HandleCloseRequest();

    auto& GetTabs() { return m_tabs; }

private:
    std::unordered_map<TabId, std::shared_ptr<ViewerTab>> m_old_tabs;
};

}  // namespace cave
