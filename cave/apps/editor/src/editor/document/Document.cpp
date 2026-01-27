#include "document.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "editor/undo_redo/UndoStack.h"

namespace cave {

OldDocument::OldDocument(const Guid& p_guid) {
    m_guid = p_guid;
    m_handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();

    m_undo_stack = std::make_unique<UndoStack>();
    m_dirty = false;
}

OldDocument::~OldDocument() {
}

bool OldDocument::CanUndo() const {
    return m_undo_stack->CanUndo();
}

bool OldDocument::CanRedo() const {
    return m_undo_stack->CanRedo();
}

void OldDocument::Undo() {
    if (m_undo_stack->Undo()) {
        m_dirty = true;
    }
}

void OldDocument::Redo() {
    if (m_undo_stack->Redo()) {
        m_dirty = true;
    }
}

bool OldDocument::Save() {
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
