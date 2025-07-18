#include "document.h"

#include "engine/runtime/asset_registry.h"
#include "editor/undo_redo/undo_stack.h"

namespace cave {

Document::Document(const Guid& p_guid) {
    m_guid = p_guid;
    m_handle = AssetRegistry::GetSingleton().FindByGuid(p_guid).unwrap();

    m_undo_stack = std::make_unique<UndoStack>();
}

}  // namespace cave
