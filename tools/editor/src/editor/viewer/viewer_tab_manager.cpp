#include "viewer_tab_manager.h"

#include "viewer_tab.h"

namespace cave {

Option<ViewerTab*> ViewerTabManager::FindTabById(const TabId& p_id) {
    auto it = m_tabs.find(p_id);
    if (it != m_tabs.end()) {
        return it->second.get();
    }
    return Option<ViewerTab*>::None();
}

Option<ViewerTab*> ViewerTabManager::FindTabByGuid(const Guid& p_guid) {
    for (const auto& [id, tab] : m_tabs) {
        if (tab->GetGuid() == p_guid) {
            return tab.get();
        }
    }
    return Option<ViewerTab*>::None();
}

Option<ViewerTab*> ViewerTabManager::GetActiveTab() {
    if (m_active_tab.is_none()) {
        return Option<ViewerTab*>::None();
    }
    return FindTabById(m_active_tab.unwrap());
}

void ViewerTabManager::SwitchTab(std::shared_ptr<ViewerTab>&& p_tab) {
    const auto& id = p_tab->GetId();
    auto [it, ok] = m_tabs.try_emplace(p_tab->GetId(), std::move(p_tab));
    DEV_ASSERT(ok);

    SwitchTab(id);
}

void ViewerTabManager::SwitchTab(const TabId& p_id) {
    if (m_active_tab == p_id) {
        return;
    }

    auto new_tab = FindTabById(p_id).unwrap();
    auto old_tab = GetActiveTab();

    if (old_tab.is_some()) {
        old_tab.unwrap()->OnDeactivate();
    }

    m_active_tab = p_id;
    m_focus_request = p_id;

    new_tab->OnActivate();

    LOG("Tool [{}] -> [{}]", old_tab.is_some() ? old_tab.unwrap()->GetTitle() : "(null)", new_tab->GetTitle());
}

void ViewerTabManager::HandleTabClose() {
    if (m_close_request.is_none()) {
        return;
    }

    std::shared_ptr<ViewerTab> to_close;

    RequestSaveDialog([&](SaveDialogResponse p_response) {
        switch (p_response) {
            case SaveDialogResponse::Save:
                // @TODO: save
                [[fallthrough]];
            case SaveDialogResponse::Discard: {
                // remove the tab
                auto it = m_tabs.find(m_close_request.unwrap());
                DEV_ASSERT(it != m_tabs.end());
                to_close = it->second;
                m_tabs.erase(it);
            } break;
            case SaveDialogResponse::Cancel:
                break;
        }

        m_close_request = Option<TabId>::None();
    });

    if (to_close) {
        to_close->OnDeactivate();
        to_close->OnDestroy();
    }
}

void ViewerTabManager::RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close) {
    ImGui::OpenPopup("Save changes to");
    if (ImGui::BeginPopupModal("Save changes to")) {
        ImGui::Text("Save changes before closing?");
        if (ImGui::Button("Save")) {
            ImGui::CloseCurrentPopup();
            p_on_close(SaveDialogResponse::Save);
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard")) {
            ImGui::CloseCurrentPopup();
            p_on_close(SaveDialogResponse::Discard);
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
            p_on_close(SaveDialogResponse::Cancel);
        }
        ImGui::EndPopup();
    }
}

}  // namespace cave