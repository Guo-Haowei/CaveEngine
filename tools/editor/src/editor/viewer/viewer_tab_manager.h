#pragma once
#include "viewer_tab_id.h"

namespace cave {

class Guid;
class ViewerTab;

class ViewerTabManager {
public:
    void SwitchTab(const TabId& p_id);
    void SwitchTab(std::shared_ptr<ViewerTab>&& p_tab);

    Option<ViewerTab*> FindTabById(const TabId& p_id);
    Option<ViewerTab*> FindTabByGuid(const Guid& p_guid);
    Option<ViewerTab*> GetActiveTab();

    void RequestClose(const TabId& p_id) { m_close_request = p_id; }
    void HandleTabClose();

    enum class SaveDialogResponse {
        Save,
        Discard,
        Cancel,
    };

    void RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close);

    const Option<TabId>& GetFocusRequest() const { return m_focus_request; }

    auto& GetTabs() { return m_tabs; }

private:
    Option<TabId> m_focus_request;
    Option<TabId> m_close_request;

    Option<TabId> m_active_tab;
    std::unordered_map<TabId, std::shared_ptr<ViewerTab>> m_tabs;
};

}  // namespace cave
