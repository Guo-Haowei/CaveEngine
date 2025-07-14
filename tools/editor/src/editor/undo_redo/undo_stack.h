#pragma once
#include "undo_command.h"

namespace my {

class UndoStack {
public:
    UndoStack(uint32_t p_max_undo = UINT_MAX)
        : m_max_undo(p_max_undo) {}

    bool Submit(std::shared_ptr<UndoCommand>&& p_command);

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
    const uint32_t m_max_undo;
    std::deque<std::shared_ptr<UndoCommand>> m_undo_stack;
    std::vector<std::shared_ptr<UndoCommand>> m_redo_stack;
};

}  // namespace my
