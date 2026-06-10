#include "Workspace.h"

#include "cave/core/diagnostics/DebugIdAllocator.h"
#include "cave/runtime/input/KeyCode.h"
#include "cave/runtime/intent/IntentDispatcher.h"

#include "engine/private/runtime/input/InputService.h"
#include "engine/private/runtime/scene/SceneRegistry.h"

#include "editor/EditorIntent.h"
#include "editor/EditorState.h"
#include "editor/services/DocumentService.h"
#include "editor/services/EditService.h"
#include "editor/services/PickingService.h"

// @TODO: delete
#include "editor/panels/SceneViewTab.h"

namespace cave {

Workspace::Workspace(EditorState& p_editor)
    : m_editor(p_editor)
    , m_debug_id(MakeDebugId(this)) {
    AppServices& services = m_editor.app().services();
    services.inputService().addConsumer(this);
    services.intentDispatcher().addHandler<OpenDocIntent>(this);
    services.intentDispatcher().addHandler<CloseDocIntent>(this);
}

Workspace::~Workspace() {
    AppServices& services = m_editor.app().services();
    services.inputService().removeConsumer(this);
    services.intentDispatcher().removeHandler<OpenDocIntent>(this);
    services.intentDispatcher().removeHandler<CloseDocIntent>(this);
}

void Workspace::Tick() {
    DrawTabs();
}

DocId Workspace::FocusedDoc() {
    Tab* tab = FocusedTab();
    return tab ? tab->docId() : DocId{};
}

PreviewScene Workspace::FocusedPreviewScene() {
    Tab* tab = FocusedTab();

    PreviewScene ret;
    if (tab) {
        ret.doc_id = tab->docId();
        ret.view_id = tab->viewId();
    }
    ret.doc_id = FocusedDoc();
    if (IDocument* doc = m_editor.DocumentService().Resolve(ret.doc_id)) {
        ret.scene_id = doc->GetPreviewScene();
        ret.scene = m_editor.app().services().sceneRegistry().resolve(ret.scene_id);
    }
    return ret;
}

void Workspace::RequestOpen(DocId p_doc_id) {
    m_editor.app().services().intentDispatcher().queue<OpenDocIntent>(p_doc_id);
}

void Workspace::RequestClose(DocId p_doc_id) {
    m_editor.app().services().intentDispatcher().queue<CloseDocIntent>(p_doc_id);
}

void Workspace::DrawTabs() {
    for (uint32_t idx = 0; idx < m_slots.size(); ++idx) {
        auto& slot = m_slots[idx];
        if (slot.storage) {
            Tab& tab = *slot.storage;
            TabId current_id = tab.tabId();
            DEV_ASSERT(current_id == TabId(idx, slot.gen));
            if (m_focused_req == current_id) {
                ImGui::SetNextWindowFocus();
                m_focused_req = TabId();
            }
            tab.DrawUI();

            if (tab.IsFocused()) {
                m_focused_tab = current_id;
            }
        }
    }
}

bool Workspace::handleIntent(Intent& p_intent) {
    if (auto open_doc = dynamic_cast<OpenDocIntent*>(&p_intent)) {
        OpenOrFocusDoc(open_doc->doc_id);
        return true;
    }

    if (auto open_doc = dynamic_cast<CloseDocIntent*>(&p_intent)) {
        CloseDoc(open_doc->doc_id);
        return true;
    }

    return false;
}

void Workspace::onEvents(const InputFrame& input) {
    if (m_editor.IsPlaying()) {
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
                m_editor.PickingService().Pick({ e.x, e.y });
                e.consumed = true;
                break;
            }
        }
    }
}

// @TODO: probably want to refactor this
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
        case AssetType::Scene:
        case AssetType::Material: {
            tab = std::make_unique<SceneViewTab>(m_editor,
                                                 p_doc_id,
                                                 doc->GetPreviewScene(),
                                                 ViewDimension::DIMENSION_3);
        } break;
        default: {
            tab = std::make_unique<Tab>(m_editor, p_doc_id);
        } break;
    }

    const TabId tab_id = Create(std::move(tab));

    Tab* tab_raw = (m_slots[tab_id.index].storage).get();
    tab_raw->tabId(tab_id);
    tab_raw->setTitleAndId(meta->name, tab_id.index);
    tab_raw->onCreate();
    m_focused_req = tab_id;
    m_doc_to_tab[p_doc_id] = tab_id;

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

bool Workspace::CloseDoc(DocId p_doc_id) {
    auto it = m_doc_to_tab.find(p_doc_id);
    DEV_ASSERT(it != m_doc_to_tab.end());

    const TabId tab_id = it->second;
    Tab* tab = Resolve(tab_id);
    DEV_ASSERT(tab->docId() == p_doc_id);
    tab->onDestroy();
    Destroy(tab_id);
    m_doc_to_tab.erase(p_doc_id);

    m_editor.DocumentService().CloseDoc(p_doc_id);
    return true;
}

extern CloseDecision AskCloseUnsaved(const char* p_title);

bool Workspace::OnCloseRequested() {
    EditService& edit = m_editor.EditService();

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