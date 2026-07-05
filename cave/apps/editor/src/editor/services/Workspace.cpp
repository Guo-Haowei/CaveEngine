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
    , engine_services_(editor.app().services())
    , editor_services_(editor.services())
    , debug_id_(MakeDebugId(this)) {
    engine_services_.inputService().addConsumer(this);
    engine_services_.intentDispatcher().addHandler<OpenDocIntent>(this);
    engine_services_.intentDispatcher().addHandler<CloseDocIntent>(this);
}

Workspace::~Workspace() {
    engine_services_.inputService().removeConsumer(this);
    engine_services_.intentDispatcher().removeHandler<OpenDocIntent>(this);
    engine_services_.intentDispatcher().removeHandler<CloseDocIntent>(this);

    refreshTabStates();
    saveWorkspaceState();
}

void Workspace::restoreProjectWorkspace() {
    ProjectManager& project_mgr = engine_services_.projectManager();
    loadWorkspaceState(project_mgr.projectRoot());

    // open documents
    for (const TabState& tab : workspace_state_.tabs) {
        if (auto handle = engine_services_.assetRegistry().findByGuid(tab.guid); handle.is_some()) {
            AssetHandle handle_ = handle.unwrap_unchecked();
            DocId doc_id = editor_services_.document().openDoc({ handle_.guid(), handle_.meta()->type });
            unused(doc_id);
        }
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
    if (IDocument* doc = editor_services_.document().resolve(ret.doc_id)) {
        ret.scene_id = doc->previewScene();
        ret.scene = engine_services_.sceneRegistry().resolve(ret.scene_id);
    }
    return ret;
}

void Workspace::requestOpen(DocId doc_id) {
    engine_services_.intentDispatcher().queue<OpenDocIntent>(doc_id);
}

void Workspace::requestClose(DocId doc_id) {
    engine_services_.intentDispatcher().queue<CloseDocIntent>(doc_id);
}

void Workspace::drawTabs() {
    for (uint32_t idx = 0; idx < slots_.size(); ++idx) {
        auto& slot = slots_[idx];
        if (slot.storage) {
            Tab& tab = *slot.storage;
            TabId current_id = tab.tabId();
            DEV_ASSERT(current_id == TabId(idx, slot.gen));
            if (request_focus_ == current_id) {
                ImGui::SetNextWindowFocus();
                request_focus_ = TabId();
            }
            tab.drawUI();

            if (tab.isFocused()) {
                focused_tab_ = current_id;
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
                editor_services_.picking().pick({ e.x, e.y });
                e.consumed = true;
                break;
            }
        }
    }
}

// @TODO: probably want to refactor this
void Workspace::openOrFocusDoc(DocId doc_id) {
    IDocument* doc = editor_services_.document().resolve(doc_id);
    if (!doc) {
        return;
    }

    const AssetMetaData* meta = doc->rawHandle().meta();
    if (!meta) {
        return;
    }

    if (auto it = doc_to_tab_.find(doc_id); it != doc_to_tab_.end()) {
        request_focus_ = it->second;
        return;
    }

    ProjectManager& project_mgr = engine_services_.projectManager();
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
    request_focus_ = tab_id;

    doc_to_tab_[doc_id] = tab_id;
    guid_to_tab_[doc->guid()] = tab_id;
}

bool Workspace::closeDoc(DocId doc_id) {
    auto it = doc_to_tab_.find(doc_id);
    DEV_ASSERT(it != doc_to_tab_.end());
    IDocument* doc = editor_services_.document().resolve(doc_id);
    Guid guid = doc ? doc->guid() : Guid::null();

    const TabId tab_id = it->second;
    Tab* tab = resolve(tab_id);
    DEV_ASSERT(tab->docId() == doc_id);
    tab->onDestroy();
    destroy(tab_id);
    doc_to_tab_.erase(it);
    guid_to_tab_.erase(guid);

    editor_services_.document().closeDoc(doc_id);
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
            if (editor_services_.edit().isDirty(doc)) {
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
        editor_services_.edit().save(doc);
    }
    return true;
}

void Workspace::onAssetChanged(const Guid&, std::span<const Guid> affected) {
    for (const Guid& guid : affected) {
        if (auto it = guid_to_tab_.find(guid); it != guid_to_tab_.end()) {
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
    auto& tabs = workspace_state_.tabs;
    tabs.clear();
    tabs.reserve(slots_.size());
    for (size_t i = 0; i < slots_.size(); ++i) {
        const Tab* tab = slots_[i].storage.get();
        if (!tab) continue;
        const DocId doc_id = tab->docId();
        const IDocument* doc = editor_services_.document().resolve(doc_id);
        if (!doc) continue;
        doc->guid();

        TabState tab_state;
        if (tab->tabState(tab_state)) {
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
            d.read(workspace_state_.content_browser.current_path);
            d.leaveKey();
        }
        d.leaveKey();
    }

    workspace_state_.tabs.clear();
    if (d.tryEnterKey("tabs")) {
        const int size = d.arraySize().unwrap_or(0);
        for (int i = 0; i < size; ++i) {
            if (d.tryEnterIndex(i)) {
                workspace_state_.tabs.resize(workspace_state_.tabs.size() + 1);
                auto& tab_state = workspace_state_.tabs.back();
                if (d.tryEnterKey("guid")) {
                    d.read(tab_state.guid);
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
    ProjectManager& project_mgr = engine_services_.projectManager();
    const fs::path path = WorkspaceFilePath(project_mgr.projectRoot());

    if (!EnsureParentDirExists(path)) {
        return;
    }

    YamlSerializer yaml;
    yaml.beginMap(false);

    yaml.beginKey("content_browser")
        .beginMap(false)
        .beginKey("current_path")
        .write(workspace_state_.content_browser.current_path)
        .endMap();

    if (!workspace_state_.tabs.empty()) {
        yaml.beginKey("tabs")
            .beginArray(false);
        for (const auto& tab : workspace_state_.tabs) {
            yaml.beginMap(false)
                .beginKey("guid")
                .write(tab.guid);
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