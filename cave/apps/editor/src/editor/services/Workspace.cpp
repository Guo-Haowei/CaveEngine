#include "Workspace.h"

#include "engine/private/runtime/framework/InputSystem.h"

#include "editor/document/DocumentService.h"
#include "editor/EditorState.h"

// @TODO: delete
#include "editor/panels/SceneViewTab.h"
//#include "engine/private/runtime/framework/ViewportManager.h"

namespace cave {

Workspace::Workspace(EditorState& p_editor)
    : m_editor(p_editor) {

    IApplication& app = m_editor.GetApp();
    app.GetInputSystem()->Router().Register(this);
}

Workspace::~Workspace() {
    IApplication& app = m_editor.GetApp();

    app.GetInputSystem()->Router().Unregister(this);
}

void Workspace::Tick() {
    FlushPendingRequests();
    DrawTabs();
}

void Workspace::Submit(WorkspaceRequest p_req) {
    m_pending_reqs.emplace_back(std::move(p_req));
}

void Workspace::FlushPendingRequests() {
    for (WorkspaceRequest& req : m_pending_reqs) {
        switch (req.type) {
            case WorkspaceRequest::Type::OpenDoc: {
                OpenOrFocusDoc(req.doc_id);
            } break;
            case WorkspaceRequest::Type::FocusDoc: {
                // OpenOrFocusDoc(req.doc_id);
            } break;
            case WorkspaceRequest::Type::CloseDoc: {
            } break;
            default:
                break;
        }
    }

    m_pending_reqs.clear();
}

void Workspace::DrawTabs() {
    for (uint32_t idx = 0; idx < m_slots.size(); ++idx) {
        auto& slot = m_slots[idx];
        if (slot.storage) {
            Tab& tab = *slot.storage;
            TabId current = TabId(idx, slot.gen);
            if (m_focused_req == current) {
                ImGui::SetNextWindowFocus();
                m_focused_req = TabId();
            }
            tab.DrawUI();

            if (tab.IsFocused()) {
                m_focused_tab = current;
            }
        }
    }
}

void Workspace::BuildViews(std::vector<SceneView>& p_out_views,
                           bool p_is_opengl) {
    for (size_t i = 0; i < m_slots.size(); ++i) {
        Tab* tab = m_slots[i].storage.get();
        if (tab && tab->IsVisible()) {
            if (SceneEditor* scene_tab = dynamic_cast<SceneEditor*>(tab)) {
                scene_tab->BuildViews(p_out_views, p_is_opengl);
            }
        }
    }
}

void Workspace::OnEvents(const std::vector<InputEvent>& p_events) {
    for (size_t i = 0; i < m_slots.size(); ++i) {
        Tab* tab = m_slots[i].storage.get();
        if (tab && tab->IsFocused()) {
            tab->OnInputEvents(p_events);
            break;
        }
    }
}

void Workspace::OpenOrFocusDoc(DocId p_doc_id) {
    IDocument* doc = m_editor.DocumentService().Resolve(p_doc_id);
    if (!doc) {
        return;
    }

    const AssetMetaData* meta = doc->GetHandleRaw().GetMeta();
    if (!meta) {
        return;
    }

    if (auto it = m_doc_to_tab.find(p_doc_id); it != m_doc_to_tab.end()) {
        m_focused_req = it->second;
        return;
    }

    std::unique_ptr<Tab> tab;
    switch (meta->type) {
        case AssetType::Scene: {
            tab = std::make_unique<SceneEditor>(m_editor,
                                                p_doc_id,
                                                doc->GetPreviewScene(),
                                                ViewDimension::DIMENSION_3);
        } break;
        default: {
            tab = std::make_unique<Tab>(m_editor,
                                        p_doc_id,
                                        ViewDimension::DIMENSION_3);
        } break;
    }

    TabId tab_id = Create(std::move(tab));

    Tab* tab_raw = (m_slots[tab_id.index].storage).get();
    tab_raw->SetTitleAndId(meta->name, tab_id.index);
    tab_raw->OnCreate();
    m_focused_req = tab_id;
    m_doc_to_tab[p_doc_id] = tab_id;

#if 0
    // @TODO: create a new tab
    std::shared_ptr<ViewerTab> tab;
    Viewer& viewer = m_editor.GetViewer();
    switch (meta->type) {
        case AssetType::Scene: {
            ViewerTab::Dimension dimension = DVAR_GET_BOOL(is_world_2d) ? ViewerTab::DIMENSION_2
                                                                        : ViewerTab::DIMENSION_3;
            tab.reset(new SceneEditor(m_editor, p_doc_id, viewer, dimension));
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
#endif
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