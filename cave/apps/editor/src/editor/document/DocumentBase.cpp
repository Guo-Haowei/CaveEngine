#include "DocumentBase.h"

namespace cave {

bool DocumentBase::Apply(std::unique_ptr<IEditCommand> p_cmd, uint32_t p_coalesce) {
    if (!p_cmd) return false;

    // Coalesce with last undo command if requested and compatible
    if (!m_undo.empty() && p_coalesce != 0 && m_last_coalesce == p_coalesce) {
        IEditCommand* last = m_undo.back().get();
        if (last && last->CanCoalesceWith(*p_cmd)) {
            // Do new change first, then merge for correct final state
            p_cmd->Do(*this);
            last->CoalesceFrom(std::move(p_cmd));
            // redo invalidated
            m_redo.clear();
            TouchDirtyAfterEdit();
            return true;
        }
    }

    p_cmd->Do(*this);
    m_undo.push_back(std::move(p_cmd));
    m_redo.clear();

    m_last_coalesce = p_coalesce;
    TouchDirtyAfterEdit();
    TrimUndoIfNeeded();
    return true;
}

bool DocumentBase::Undo() {
    if (m_undo.empty()) return false;
    auto cmd = std::move(m_undo.back());
    m_undo.pop_back();

    cmd->Undo(*this);
    m_redo.push_back(std::move(cmd));

    m_last_coalesce = 0;
    RecomputeDirtyAfterHistoryMove();
    return true;
}

bool DocumentBase::Redo() {
    if (m_redo.empty()) return false;
    auto cmd = std::move(m_redo.back());
    m_redo.pop_back();

    cmd->Do(*this);
    m_undo.push_back(std::move(cmd));

    m_last_coalesce = 0;
    RecomputeDirtyAfterHistoryMove();
    return true;
}

void DocumentBase::GetUndoLabels(std::vector<std::string>& p_out, int p_max_items) const {
    p_out.clear();
    int count = 0;
    for (auto it = m_undo.rbegin(); it != m_undo.rend() && count < p_max_items; ++it, ++count) {
        p_out.emplace_back((*it)->Label());
    }
}

void DocumentBase::GetRedoLabels(std::vector<std::string>& p_out, int p_max_items) const {
    p_out.clear();
    int count = 0;
    for (auto it = m_redo.rbegin(); it != m_redo.rend() && count < p_max_items; ++it, ++count) {
        p_out.emplace_back((*it)->Label());
    }
}

void DocumentBase::TrimUndoIfNeeded() {
    if (m_undo_limit == 0) return;
    while (m_undo.size() > m_undo_limit) {
        // If we drop history older than the save marker,
        // we must shift the marker accordingly to preserve meaning.
        m_undo.pop_front();
        if (m_saved_undo_size > 0) {
            m_saved_undo_size -= 1;
        }
    }
}

}  // namespace cave
