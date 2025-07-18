#pragma once
#include "engine/assets/asset_handle.h"

namespace cave {

class UndoStack;

class Document {
public:
    Document(const Guid& p_guid);

    UndoStack& GetUndo() {
        return *(m_undo_stack.get());
    }

protected:
    Guid m_guid;
    AssetHandle m_handle;

    std::unique_ptr<UndoStack> m_undo_stack;

};

}  // namespace cave
