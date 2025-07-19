#include "document.h"

#include "engine/runtime/asset_registry.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

Document::Document(const Guid& p_guid) {
    m_guid = p_guid;
    m_handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();

    m_undo_stack = std::make_unique<UndoStack>();
    m_dirty = false;
}

bool Document::CanUndo() const {
    return m_undo_stack->CanUndo();
}

bool Document::CanRedo() const {
    return m_undo_stack->CanUndo();
}

void Document::Undo() {
    if (m_undo_stack->Undo()) {
        m_dirty = true;
    }
}

void Document::Redo() {
    if (m_undo_stack->Redo()) {
        m_dirty = true;
    }
}

bool Document::Save() {
    // @TODO: instead of write directly to the asset,
    // create a tmp asset and writes to it,
    // replace the original asset content with tmp asset on save
    if (m_dirty) {
        AssetRegistry::GetSingleton().SaveAsset(m_guid);
        m_dirty = false;
        return true;
    }

    return false;
}

}  // namespace cave
