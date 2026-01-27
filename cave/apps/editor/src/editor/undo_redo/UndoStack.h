#pragma once
#include "UndoCommand.h"

namespace cave {

class UndoStack {
public:
    bool Submit(std::unique_ptr<UndoCommand> p_command);

    bool Undo();
    bool Redo();

    bool CanUndo() const {
        return !m_undo_stack.empty();
    }

    bool CanRedo() const {
        return !m_redo_stack.empty();
    }

    void Clear() {
        m_undo_stack.clear();
        m_redo_stack.clear();
    }

private:
    std::deque<std::unique_ptr<UndoCommand>> m_undo_stack;
    std::vector<std::unique_ptr<UndoCommand>> m_redo_stack;
};

}  // namespace cave
