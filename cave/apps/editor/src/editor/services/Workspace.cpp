#include "Workspace.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/input/KeyCode.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/projects/ProjectManager.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorIntent.h"
#include "editor/EditorState.h"
#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"

// @TODO: delete
#include "editor/panels/SceneViewTab.h"

namespace cave {

Workspace::Workspace(EditorState& editor)
    : editor_(editor)
    , services_(editor.app().services())
    , debug_id_(MakeDebugId(this)) {
    services_.inputService().addConsumer(this);
    services_.intentDispatcher().addHandler<OpenDocIntent>(this);
    services_.intentDispatcher().addHandler<CloseDocIntent>(this);
}

Workspace::~Workspace() {
    services_.inputService().removeConsumer(this);
    services_.intentDispatcher().removeHandler<OpenDocIntent>(this);
    services_.intentDispatcher().removeHandler<CloseDocIntent>(this);
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
    if (IDocument* doc = editor_.DocumentService().Resolve(ret.doc_id)) {
        ret.scene_id = doc->GetPreviewScene();
        ret.scene = services_.sceneRegistry().resolve(ret.scene_id);
    }
    return ret;
}

void Workspace::requestOpen(DocId doc_id) {
    services_.intentDispatcher().queue<OpenDocIntent>(doc_id);
}

void Workspace::requestClose(DocId doc_id) {
    services_.intentDispatcher().queue<CloseDocIntent>(doc_id);
}

void Workspace::drawTabs() {
    for (uint32_t idx = 0; idx < m_slots.size(); ++idx) {
        auto& slot = m_slots[idx];
        if (slot.storage) {
            Tab& tab = *slot.storage;
            TabId current_id = tab.tabId();
            DEV_ASSERT(current_id == TabId(idx, slot.gen));
            if (request_focus_ == current_id) {
                ImGui::SetNextWindowFocus();
                request_focus_ = TabId();
            }
            tab.drawUI();

            if (tab.IsFocused()) {
                focused_tab_ = current_id;
            }
        }
    }
}

bool Workspace::handleIntent(Intent& intent) {
    if (auto open_doc = dynamic_cast<OpenDocIntent*>(&intent)) {
        openOrFocusDoc(open_doc->doc_id);
        return true;
    }

    if (auto close_doc = dynamic_cast<CloseDocIntent*>(&intent)) {
        closeDoc(close_doc->doc_id);
        return true;
    }

    return false;
}

void Workspace::onEvents(const InputFrame& input) {
    if (editor_.IsPlaying()) {
        return;
    }

    for (size_t i = 0; i < m_slots.size(); ++i) {
        Tab* tab = m_slots[i].storage.get();
        if (tab && tab->IsHovered()) {
            tab->onInputEvents(input);
            break;
        }
    }

    for (const InputEvent& e : input.events) {
        if (e.consumed) continue;
        if (e.type == InputEventType::ButtonDown) {
            const Key key = static_cast<Key>(e.code);
            if (key == Key::RMB) {
                editor_.PickingService().Pick({ e.x, e.y });
                e.consumed = true;
                break;
            }
        }
    }
}

// @TODO: probably want to refactor this
void Workspace::openOrFocusDoc(DocId doc_id) {
    IDocument* doc = editor_.DocumentService().Resolve(doc_id);
    if (!doc) {
        return;
    }

    const AssetMetaData* meta = doc->GetHandleRaw().GetMeta();
    if (!meta) {
        return;
    }

    if (auto it = doc_to_tab_.find(doc_id); it != doc_to_tab_.end()) {
        request_focus_ = it->second;
        return;
    }

    ProjectManager& project_mgr = services_.projectManager();
    const ViewDimension dim = project_mgr.project().is_2d ? ViewDimension::Dim2 : ViewDimension::Dim3;

    std::unique_ptr<Tab> tab;
    switch (meta->type) {
        case AssetType::Scene:
        case AssetType::Material: {
            tab = std::make_unique<SceneViewTab>(editor_,
                                                 doc_id,
                                                 doc->GetPreviewScene(),
                                                 dim);
        } break;
        default: {
            tab = std::make_unique<Tab>(editor_, doc_id);
        } break;
    }

    const TabId tab_id = Create(std::move(tab));

    Tab* tab_raw = (m_slots[tab_id.index].storage).get();
    tab_raw->tabId(tab_id);
    tab_raw->setTitleAndId(meta->name, tab_id.index);
    tab_raw->onCreate();
    request_focus_ = tab_id;
    doc_to_tab_[doc_id] = tab_id;

#if 0
    // @TODO: create a new tab
    switch (meta->type) {
        case AssetType::Scene: {
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
#endif
}

bool Workspace::closeDoc(DocId doc_id) {
    auto it = doc_to_tab_.find(doc_id);
    DEV_ASSERT(it != doc_to_tab_.end());

    const TabId tab_id = it->second;
    Tab* tab = Resolve(tab_id);
    DEV_ASSERT(tab->docId() == doc_id);
    tab->onDestroy();
    Destroy(tab_id);
    doc_to_tab_.erase(doc_id);

    editor_.DocumentService().CloseDoc(doc_id);
    return true;
}

// @TODO: refactor this part
extern CloseDecision AskCloseUnsaved(const char* title);

bool Workspace::onCloseRequested() {
    EditService& edit = editor_.EditService();

    std::vector<DocId> unsaved;
    for (uint32_t idx = 0; idx < m_slots.size(); ++idx) {
        auto& slot = m_slots[idx];
        if (slot.storage) {
            Tab& tab = *slot.storage;
            DocId doc = tab.docId();
            if (edit.IsDirty(doc)) {
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
        edit.Save(doc);
    }
    return true;
}

}  // namespace cave