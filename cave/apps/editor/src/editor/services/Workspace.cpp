#include "Workspace.h"

#include "editor/document/DocumentService.h"
#include "editor/EditorState.h"

// @TODO: delete
#include "editor/viewer/ViewerTab.h"
#include "editor/document/document.h"
#include "editor/EditorDvars.h"
#include "editor/scene_editor/SceneEditor.h"
#include "engine/private/runtime/framework/ViewportManager.h"

namespace cave {

Workspace::Workspace(EditorState& p_editor)
    : m_editor(p_editor) {
}

void Workspace::SendRequest(WorkspaceRequest p_request) {
    m_pending_reqs.emplace_back(std::move(p_request));
}

void Workspace::Tick(float p_dt) {
    unused(p_dt);

    for (WorkspaceRequest& req : m_pending_reqs) {
        switch (req.type) {
            case WorkspaceRequest::Type::OpenDoc: {
                OpenOrFocusDoc(req.doc_id);
            } break;
            case WorkspaceRequest::Type::FocusDoc: {
                // OpenOrFocusDoc(req.doc_id);
            } break;
            default:
                break;
        }
    }

    m_pending_reqs.clear();
}
void Workspace::OpenOrFocusDoc(DocId p_doc_id) {
    if (auto it = m_tabs.find(p_doc_id); it != m_tabs.end()) {
        DEV_ASSERT(0);
        return;
    }

    IDocument* doc = m_editor.GetDocumentService().Resolve(p_doc_id);
    if (!doc) {
        return;
    }

    const AssetMetaData* meta = doc->GetHandleRaw().GetMeta();
    if (!meta) {
        return;
    }

    // @TODO: create a new tab
    std::shared_ptr<ViewerTab> tab;
    Viewer& viewer = m_editor.GetViewer();
    switch (meta->type) {
        case AssetType::Scene: {
            ViewerTab::Dimension dimension = DVAR_GET_BOOL(is_world_2d) ? ViewerTab::DIMENSION_2
                                                                        : ViewerTab::DIMENSION_3;
            tab.reset(new SceneEditor(m_editor, viewer, dimension));
        } break;
        // case AssetType::TileSet: {
        //     tab.reset(new TileSetEditor(m_editor, *this));
        // } break;
        // case AssetType::TileMap: {
        //     tab.reset(new TileMapEditor(m_editor, *this));
        // } break;
        // case AssetType::SpriteAnimation: {
        //     tab.reset(new SpriteAnimationEditor(m_editor, *this));
        // } break;
        // case AssetType::Material: {
        //     tab.reset(new MaterialEditor(m_editor, *this));
        // } break;
        default: {
            CRASH_NOW_MSG("not supported");
        } break;
    }

    ViewportManager* viewport_manager = m_editor.GetApp().GetViewportManager();
    viewport_manager->CreateViewport(tab);

    // DVAR_SET_STRING(last_open_asset, p_guid.ToString());
    tab->OnCreate(meta->guid);
    tab->OnActivate();
    // @TODO: set active tab
    LOG_VERBOSE("tab {} created", tab->GetTitle());

    m_tabs[p_doc_id] = tab;
    m_active_tab = tab.get();
}

//-------------- DEPRECATE ------------------

void Workspace::HandleCloseRequest() {
    // if (m_close_request.is_none()) {
    //     return;
    // }

#if 0
    RequestSaveDialog([&](SaveDialogResponse p_response) {
        auto it = m_old_tabs.find(m_close_request.unwrap());
        std::shared_ptr<ViewerTab> to_close = it->second;
        DEV_ASSERT(it != m_old_tabs.end());
        switch (p_response) {
            case SaveDialogResponse::Save:
                to_close->GetDocument().Save();
                // @TODO: save
                [[fallthrough]];
            case SaveDialogResponse::Discard: {
                // remove the tab
                m_old_tabs.erase(it);
                to_close->OnDeactivate();
                to_close->OnDestroy();
            } break;
            case SaveDialogResponse::Cancel:
                break;
        }

        m_close_request = None();
    });
#endif
}

void Workspace::RequestSaveDialog(std::function<void(SaveDialogResponse)> p_on_close) {
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