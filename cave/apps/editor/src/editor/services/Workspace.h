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

class Workspace {
public:
    Workspace(EditorState& p_editor);

    void OpenOrFocusDoc(DocId doc_id);

    bool RequestCloseDoc(DocId doc_id);

    bool RequestCloseTab(TabId tab_id);

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
private:
    EditorState& m_editor;

    //----------------------------------------------------------------
    // @TODO: deprecate below apis
public:
    void SwitchTab(const TabId& p_id);
    void SwitchTab(std::shared_ptr<ViewerTab>&& p_tab);

    Option<ViewerTab*> FindTabById(const TabId& p_id);
    Option<ViewerTab*> FindTabByGuid(const Guid& p_guid);
    Option<ViewerTab*> GetActiveTab();

    void RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close);

    void SetCloseRequest(const TabId& p_id) { m_close_request = Some(p_id); }
    void HandleCloseRequest();

    const Option<TabId>& GetFocusRequest() const { return m_focus_request; }
    void ClearFocusRequest() { m_focus_request = None(); }

    auto& GetTabs() { return m_tabs; }

private:
    Option<TabId> m_focus_request{ None() };
    Option<TabId> m_close_request{ None() };

    Option<TabId> m_active_tab;
    std::unordered_map<TabId, std::shared_ptr<ViewerTab>> m_tabs;
};

}  // namespace cave
