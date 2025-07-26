#include "viewer_tab_manager.h"

#include "editor/document/document.h"
#include "editor/viewer/viewer_tab.h"

namespace cave {

Option<ViewerTab*> ViewerTabManager::FindTabById(const TabId& p_id) {
    auto it = m_tabs.find(p_id);
    if (it != m_tabs.end()) {
        return Some(it->second.get());
    }
    return None();
}

Option<ViewerTab*> ViewerTabManager::FindTabByGuid(const Guid& p_guid) {
    for (const auto& [id, tab] : m_tabs) {
        if (tab->GetGuid() == p_guid) {
            return Some(tab.get());
        }
    }
    return None();
}

Option<ViewerTab*> ViewerTabManager::GetActiveTab() {
    if (m_active_tab.is_none()) {
        return None();
    }
    return FindTabById(m_active_tab.unwrap_unchecked());
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
        old_tab.unwrap_unchecked()->OnDeactivate();
    }

    m_active_tab = Some(p_id);
    m_focus_request = Some(p_id);

    new_tab->OnActivate();

    LOG("Tool [{}] -> [{}]", old_tab.is_some() ? old_tab.unwrap_unchecked()->GetTitle() : "(null)", new_tab->GetTitle());
}

void ViewerTabManager::HandleCloseRequest() {
    if (m_close_request.is_none()) {
        return;
    }

    RequestSaveDialog([&](SaveDialogResponse p_response) {
        auto it = m_tabs.find(m_close_request.unwrap());
        std::shared_ptr<ViewerTab> to_close = it->second;
        DEV_ASSERT(it != m_tabs.end());
        switch (p_response) {
            case SaveDialogResponse::Save:
                to_close->GetDocument().Save();
                // @TODO: save
                [[fallthrough]];
            case SaveDialogResponse::Discard: {
                // remove the tab
                m_tabs.erase(it);
                to_close->OnDeactivate();
                to_close->OnDestroy();
            } break;
            case SaveDialogResponse::Cancel:
                break;
        }

        m_close_request = None();
    });
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