#include "Workspace.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/input/KeyCode.h"
#include "cave/runtime/intent/IntentBus.h"

#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/projects/ProjectManager.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorIntent.h"
#include "editor/EditorState.h"
#include "editor/animation_editor/SpriteAnimationEditor.h"
#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"
#include "editor/tile_map/TileMapEditor.h"
#include "editor/tile_map/TileSetEditor.h"

// @TODO: refactor
#include "editor/panels/SceneViewTab.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

namespace fs = std::filesystem;

Workspace::Workspace(EditorState& editor)
    : m_editor(editor)
    , m_engine_services(editor.app().services())
    , m_editor_services(editor.services())
    , m_debug_id(MakeDebugId(this)) {
    m_engine_services.inputService().addConsumer(this);
    m_engine_services.intentBus().addHandler<OpenDocIntent>(this);
    m_engine_services.intentBus().addHandler<CloseDocIntent>(this);

    if (buildStateCachePath()) {
        loadWorkspaceState();
    }
}

Workspace::~Workspace() {
    m_engine_services.inputService().removeConsumer(this);
    m_engine_services.intentBus().removeHandler<OpenDocIntent>(this);
    m_engine_services.intentBus().removeHandler<CloseDocIntent>(this);

    m_workspace_state.saveNow(m_workspace_file);
}

void Workspace::restoreTabs() {
    DocumentService& document = m_editor_services.document();
    Option<OpenDocDesc> active_doc;
    for (const TabState& tab : m_workspace_state.tabs) {
        if (auto handle = m_engine_services.assetRegistry().findByGuid(tab.guid)) {
            AssetHandle handle_ = handle.unwrap_unchecked();
            OpenDocDesc desc{ handle_.guid(), handle_.meta()->type };
            if (tab.active && active_doc.is_none()) {
                active_doc = Some(desc);
                continue;
            }
            document.openDoc(desc);
        }
    }

    if (active_doc.is_some()) {
        document.openDoc(active_doc.unwrap_unchecked());
    }
}

void Workspace::tick(float dt) {
    drawTabs();

    refreshTabStates();
    saveWorkspaceState(dt);
}

DocId Workspace::focusedDoc() {
    Tab* tab = focusedTab();
    return tab ? tab->docId() : DocId{};
}

PreviewScene Workspace::focusedPreviewScene() {
    Tab* tab = focusedTab();

    PreviewScene ret;
    if (tab) {
        ret.doc_id = tab->docId();
        ret.view_id = tab->viewId();
    }
    if (IDocument* doc = m_editor_services.document().resolve(ret.doc_id)) {
        ret.scene_id = doc->previewScene();
        ret.scene = m_engine_services.sceneRegistry().resolve(ret.scene_id);
        ret.guid = doc->guid();
    }
    return ret;
}

void Workspace::requestOpen(DocId doc_id) {
    m_engine_services.intentBus().queue<OpenDocIntent>(doc_id);
}

void Workspace::requestClose(DocId doc_id) {
    m_engine_services.intentBus().queue<CloseDocIntent>(doc_id);
}

void Workspace::drawTabs() {
    for (uint32_t idx = 0; idx < m_slots.size(); ++idx) {
        auto& slot = m_slots[idx];
        if (slot.storage) {
            Tab& tab = *slot.storage;
            TabId current_id = tab.tabId();
            DEV_ASSERT(current_id == TabId(idx, slot.gen));
            if (m_request_focus == current_id) {
                ImGui::SetNextWindowFocus();
                m_request_focus = TabId();
            }
            tab.drawUI();

            if (tab.isFocused()) {
                m_focused_tab = current_id;
            }
        }
    }
}

bool Workspace::handleIntent(Intent& intent) {
    if (auto open_doc = dynamic_cast<OpenDocIntent*>(&intent)) {
        openOrFocusDoc(open_doc->doc_id());
        return true;
    }

    if (auto close_doc = dynamic_cast<CloseDocIntent*>(&intent)) {
        closeDoc(close_doc->doc_id());
        return true;
    }

    return false;
}

void Workspace::onEvents(const InputFrame& input) {
    if (m_editor.isPlaying()) {
        return;
    }

    for (size_t i = 0; i < m_slots.size(); ++i) {
        Tab* tab = m_slots[i].storage.get();
        if (tab && tab->isHovered()) {
            tab->onInputEvents(input);
            break;
        }
    }

    for (const InputEvent& e : input.events) {
        if (e.consumed) continue;
        if (e.type == InputEventType::ButtonDown) {
            const Key key = static_cast<Key>(e.code);
            if (key == Key::RMB) {
                m_editor_services.picking().pick({ e.x, e.y });
                e.consumed = true;
                break;
            }
        }
    }
}

// @TODO: probably want to refactor this
void Workspace::openOrFocusDoc(DocId doc_id) {
    IDocument* doc = m_editor_services.document().resolve(doc_id);
    if (!doc) {
        return;
    }

    const AssetMetaData* meta = doc->rawHandle().meta();
    if (!meta) {
        return;
    }

    if (auto it = m_doc_to_tab.find(doc_id); it != m_doc_to_tab.end()) {
        m_request_focus = it->second;
        return;
    }

    ProjectManager& project_mgr = m_engine_services.projectManager();
    const ViewDimension dim = project_mgr.project().is_2d ? ViewDimension::Dim2 : ViewDimension::Dim3;

    Owner<Tab> tab;
    switch (meta->type) {
        case AssetType::Material:
        case AssetType::Prefab:
        case AssetType::Scene: {
            tab = std::make_unique<SceneViewTab>(m_editor,
                                                 doc_id,
                                                 doc->previewScene(),
                                                 dim);
        } break;
        case AssetType::TileMap: {
            tab = std::make_unique<TileMapEditor>(m_editor, doc_id, doc->previewScene());
        } break;
        case AssetType::TileSet: {
            tab = std::make_unique<TileSetEditor>(m_editor, doc_id);
        } break;
        case AssetType::SpriteAnimation: {
            tab = std::make_unique<SpriteAnimationEditor>(m_editor, doc_id, doc->previewScene());
        } break;
        default: {
            tab = std::make_unique<Tab>(m_editor, doc_id);
        } break;
    }

    const TabId tab_id = create(std::move(tab));

    Tab* tab_raw = (m_slots[tab_id.index].storage).get();
    tab_raw->tabId(tab_id);
    tab_raw->onCreate();
    m_request_focus = tab_id;

    m_doc_to_tab[doc_id] = tab_id;
    m_guid_to_tab[doc->guid()] = tab_id;
}

bool Workspace::closeDoc(DocId doc_id) {
    auto it = m_doc_to_tab.find(doc_id);
    DEV_ASSERT(it != m_doc_to_tab.end());
    IDocument* doc = m_editor_services.document().resolve(doc_id);
    Guid guid = doc ? doc->guid() : Guid::null();

    const TabId tab_id = it->second;
    Tab* tab = resolve(tab_id);
    DEV_ASSERT(tab->docId() == doc_id);
    tab->onDestroy();
    destroy(tab_id);
    m_doc_to_tab.erase(it);
    m_guid_to_tab.erase(guid);

    m_editor_services.document().closeDoc(doc_id);
    return true;
}

// @TODO: refactor this part
extern CloseDecision AskCloseUnsaved(const char* title);

bool Workspace::onCloseRequested() {
    std::vector<DocId> unsaved;
    for (uint32_t idx = 0; idx < m_slots.size(); ++idx) {
        auto& slot = m_slots[idx];
        if (slot.storage) {
            Tab& tab = *slot.storage;
            DocId doc = tab.docId();
            if (m_editor_services.edit().isDirty(doc)) {
                unsaved.push_back(doc);
            }
        }
    }

    if (unsaved.empty()) {
        return true;
    }

    CloseDecision desicion = AskCloseUnsaved("Warning");
    switch (desicion) {
        case CloseDecision::Save:
            break;
        case CloseDecision::Discard:
            return true;
        case CloseDecision::Cancel:
            return false;
    }
    for (DocId doc : unsaved) {
        m_editor_services.edit().save(doc);
    }
    return true;
}

void Workspace::onAssetChanged(const Guid&, std::span<const Guid> affected) {
    for (const Guid& guid : affected) {
        if (auto it = m_guid_to_tab.find(guid); it != m_guid_to_tab.end()) {
            const TabId tab_id = it->second;
            if (auto tab = dynamic_cast<ViewTabBase*>(resolve(tab_id))) {
                tab->requestSceneReload();
            }
        }
    }
}

void Workspace::refreshTabStates() {
    auto& tabs = m_workspace_state.tabs;
    tabs.clear();
    tabs.reserve(m_slots.size());
    for (uint32_t i = 0; i < (uint32_t)m_slots.size(); ++i) {
        const Tab* tab = m_slots[i].storage.get();
        if (!tab) continue;
        const DocId doc_id = tab->docId();
        const IDocument* doc = m_editor_services.document().resolve(doc_id);
        if (!doc) continue;
        doc->guid();

        TabState tab_state;
        if (tab->tabState(tab_state)) {
            tab_state.active = m_focused_tab.index == i;
            tabs.emplace_back(std::move(tab_state));
        }
    }

    // @TODO: proper dirty tracking
    m_workspace_state.markDirty();
}

bool Workspace::buildStateCachePath() {
    ProjectManager& project_mgr = m_engine_services.projectManager();
    std::string project_root = project_mgr.projectRoot();
    if (DEV_VERIFY(!project_root.empty())) {
        m_workspace_file = fs::path{ project_root } / ".cave" / "workspace.yaml";
    }

    return true;
}

bool Workspace::loadWorkspaceState() {
    if (m_workspace_file.empty()) {
        return false;
    }

    return m_workspace_state.load(m_workspace_file);
}

void Workspace::saveWorkspaceState(float dt) {
    if (m_workspace_file.empty()) {
        return;
    }

    if (!m_workspace_state.save(m_workspace_file, dt)) {
        LOG_WARN(LogChannel::FS, "failed to save '{}'", m_workspace_file.string());
    }
}

}  // namespace cave