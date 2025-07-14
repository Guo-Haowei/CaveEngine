#include "undo_stack.h"

namespace my {

bool UndoStack::Submit(std::shared_ptr<UndoCommand>&& p_command) {
    // New commands invalidate redo history
    if (!m_redo_stack.empty()) {
        m_redo_stack.clear();
    }

    // Try merge the command
    if (!m_undo_stack.empty()) {
        auto last = m_undo_stack.back();
        if (last->MergeCommand(p_command.get())) {
            return true;
        }
    }

    // If can't merge, add to undo stack
    m_undo_stack.emplace_back(std::move(p_command));
    return true;
}

bool UndoStack::Undo() {
    if (!CanUndo()) {
        return false;
    }

    auto cmd = std::move(m_undo_stack.back());
    m_undo_stack.pop_back();

    cmd->Undo();
    m_redo_stack.emplace_back(std::move(cmd));
    return true;
}

bool UndoStack::Redo() {
    if (!CanRedo()) {
        return false;
    }

    auto cmd = std::move(m_redo_stack.back());
    m_redo_stack.pop_back();

    cmd->Redo();
    m_undo_stack.emplace_back(std::move(cmd));
    return true;
}

}  // namespace my
