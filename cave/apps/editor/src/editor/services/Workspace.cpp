#include "Workspace.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/input/KeyCode.h"
#include "cave/runtime/intent/IntentDispatcher.h"

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
#include "engine/private/runtime/serialization/YamlInclude.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

namespace fs = std::filesystem;

Workspace::Workspace(EditorState& editor)
    : editor_(editor)
    , m_engine_services(editor.app().services())
    , m_editor_services(editor.services())
    , m_debug_id(MakeDebugId(this)) {
    m_engine_services.inputService().addConsumer(this);
    m_engine_services.intentDispatcher().addHandler<OpenDocIntent>(this);
    m_engine_services.intentDispatcher().addHandler<CloseDocIntent>(this);
}

Workspace::~Workspace() {
    m_engine_services.inputService().removeConsumer(this);
    m_engine_services.intentDispatcher().removeHandler<OpenDocIntent>(this);
    m_engine_services.intentDispatcher().removeHandler<CloseDocIntent>(this);

    refreshTabStates();
    saveWorkspaceState();
}

void Workspace::restoreProjectWorkspace() {
    ProjectManager& project_mgr = m_engine_services.projectManager();
    loadWorkspaceState(project_mgr.projectRoot());

    // open documents
    Option<DocId> active_doc_id;
    for (const TabState& tab : m_workspace_state.tabs) {
        if (auto handle = m_engine_services.assetRegistry().findByGuid(tab.guid); handle.is_some()) {
            AssetHandle handle_ = handle.unwrap_unchecked();
            DocId doc_id = m_editor_services.document().openDoc({ handle_.guid(), handle_.meta()->type });
            if (tab.active) {
                active_doc_id = Some(doc_id);
            }
        }
    }

    if (active_doc_id.is_some()) {
        requestOpen(active_doc_id.unwrap_unchecked());
    }
}

void Workspace::tick() {
    drawTabs();
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
    ret.doc_id = focusedDoc();
    if (IDocument* doc = m_editor_services.document().resolve(ret.doc_id)) {
        ret.scene_id = doc->previewScene();
        ret.scene = m_engine_services.sceneRegistry().resolve(ret.scene_id);
    }
    return ret;
}

void Workspace::requestOpen(DocId doc_id) {
    m_engine_services.intentDispatcher().queue<OpenDocIntent>(doc_id);
}

void Workspace::requestClose(DocId doc_id) {
    m_engine_services.intentDispatcher().queue<CloseDocIntent>(doc_id);
}

void Workspace::drawTabs() {
    for (uint32_t idx = 0; idx < slots_.size(); ++idx) {
        auto& slot = slots_[idx];
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
    if (editor_.isPlaying()) {
        return;
    }

    for (size_t i = 0; i < slots_.size(); ++i) {
        Tab* tab = slots_[i].storage.get();
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

    std::unique_ptr<Tab> tab;
    switch (meta->type) {
        case AssetType::Scene:
        case AssetType::Material: {
            tab = std::make_unique<SceneViewTab>(editor_,
                                                 doc_id,
                                                 doc->previewScene(),
                                                 dim);
        } break;
        case AssetType::TileMap: {
            tab = std::make_unique<TileMapEditor>(editor_, doc_id, doc->previewScene());
        } break;
        case AssetType::TileSet: {
            tab = std::make_unique<TileSetEditor>(editor_, doc_id);
        } break;
        case AssetType::SpriteAnimation: {
            tab = std::make_unique<SpriteAnimationEditor>(editor_, doc_id, doc->previewScene());
        } break;
        default: {
            tab = std::make_unique<Tab>(editor_, doc_id);
        } break;
    }

    const TabId tab_id = create(std::move(tab));

    Tab* tab_raw = (slots_[tab_id.index].storage).get();
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
    for (uint32_t idx = 0; idx < slots_.size(); ++idx) {
        auto& slot = slots_[idx];
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

namespace {

fs::path WorkspaceFilePath(std::string_view project_root) {
    DEV_ASSERT(!project_root.empty());
    return fs::path{ project_root } / ".cave" / "workspace.yaml";
}

bool EnsureParentDirExists(const fs::path& file_path) {
    std::error_code ec;
    fs::create_directories(file_path.parent_path(), ec);

    if (ec) {
        LOG_ERROR("Failed to create directory '{}': {}",
                  file_path.parent_path().string(),
                  ec.message());
        return false;
    }

    return true;
}

}  // namespace

void Workspace::refreshTabStates() {
    auto& tabs = m_workspace_state.tabs;
    tabs.clear();
    tabs.reserve(slots_.size());
    for (uint32_t i = 0; i < (uint32_t)slots_.size(); ++i) {
        const Tab* tab = slots_[i].storage.get();
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
}

bool Workspace::loadWorkspaceState(std::string_view project_root) {
    if (project_root.empty()) {
        return false;
    }

    const fs::path path = WorkspaceFilePath(project_root);

    if (!fs::exists(path)) {
        return true;
    }

    YAML::Node root;
    if (auto res = LoadYaml(path.string(), root); !res) {
        LOG_ERROR(LogChannel::FS, "{}", ToString(res.error()));
        return false;
    }

    YamlDeserializer yaml;
    yaml.Initialize(root);
    IDeserializer& d = yaml;

    if (d.tryEnterKey("content_browser")) {
        if (d.tryEnterKey("current_path")) {
            d.read(m_workspace_state.content_browser.current_path);
            d.leaveKey();
        }
        d.leaveKey();
    }

    m_workspace_state.tabs.clear();
    if (d.tryEnterKey("tabs")) {
        const int size = d.arraySize().unwrap_or(0);
        for (int i = 0; i < size; ++i) {
            if (d.tryEnterIndex(i)) {
                m_workspace_state.tabs.resize(m_workspace_state.tabs.size() + 1);
                auto& tab_state = m_workspace_state.tabs.back();
                if (d.tryEnterKey("guid")) {
                    d.read(tab_state.guid);
                    d.leaveKey();
                }

                if (d.tryEnterKey("active")) {
                    d.read(tab_state.active);
                    d.leaveKey();
                }

                TransformComponent transform;
                if (d.tryEnterKey("transform")) {
                    if (d.read(transform)) {
                        tab_state.transform = Some(transform);
                    }
                    d.leaveKey();
                }

                CameraComponent camera;
                if (d.tryEnterKey("camera")) {
                    if (d.read(camera)) {
                        tab_state.camera = Some(camera);
                    }
                    d.leaveKey();
                }

                d.leaveIndex();
            }
        }
        d.leaveKey();
    }

    return true;
}

void Workspace::saveWorkspaceState() {
    ProjectManager& project_mgr = m_engine_services.projectManager();
    const fs::path path = WorkspaceFilePath(project_mgr.projectRoot());

    if (!EnsureParentDirExists(path)) {
        return;
    }

    YamlSerializer yaml;
    yaml.beginMap(false);

    yaml.beginKey("content_browser")
        .beginMap(false)
        .beginKey("current_path")
        .write(m_workspace_state.content_browser.current_path)
        .endMap();

    if (!m_workspace_state.tabs.empty()) {
        yaml.beginKey("tabs")
            .beginArray(false);
        for (const auto& tab : m_workspace_state.tabs) {
            yaml.beginMap(false)
                .beginKey("guid")
                .write(tab.guid);
            if (tab.active) {
                yaml.beginKey("active").write(tab.active);
            }
            if (tab.camera.is_some()) {
                yaml.beginKey("camera").write(tab.camera.unwrap_unchecked());
            }
            if (tab.transform.is_some()) {
                yaml.beginKey("transform").write(tab.transform.unwrap_unchecked());
            }
            yaml.endMap();
        }
        yaml.endArray();
    }

    yaml.endMap();

    if (auto res = SaveYaml(path.string(), yaml); !res) {
        LOG_ERROR(LogChannel::FS, "{}", ToString(res.error()));
    }
}

}  // namespace cave