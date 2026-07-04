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

// @TODO: delete
#include "editor/panels/SceneViewTab.h"

namespace cave {

Workspace::Workspace(EditorState& editor)
    : editor_(editor)
    , app_services_(editor.app().services())
    , editor_services_(editor.services())
    , debug_id_(MakeDebugId(this)) {
    app_services_.inputService().addConsumer(this);
    app_services_.intentDispatcher().addHandler<OpenDocIntent>(this);
    app_services_.intentDispatcher().addHandler<CloseDocIntent>(this);
}

Workspace::~Workspace() {
    app_services_.inputService().removeConsumer(this);
    app_services_.intentDispatcher().removeHandler<OpenDocIntent>(this);
    app_services_.intentDispatcher().removeHandler<CloseDocIntent>(this);
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
        ret.scene = app_services_.sceneRegistry().resolve(ret.scene_id);
    }
    return ret;
}

void Workspace::requestOpen(DocId doc_id) {
    app_services_.intentDispatcher().queue<OpenDocIntent>(doc_id);
}

void Workspace::requestClose(DocId doc_id) {
    app_services_.intentDispatcher().queue<CloseDocIntent>(doc_id);
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
    if (editor_.IsPlaying()) {
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

    ProjectManager& project_mgr = app_services_.projectManager();
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
                tab->requestReload();
            }
        }
    }
}

}  // namespace cave