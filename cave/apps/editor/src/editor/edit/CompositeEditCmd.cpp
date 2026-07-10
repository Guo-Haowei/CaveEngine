#include "CompositeEditCmd.h"

namespace cave {

void CompositeEditCmd::AddCommand(Owner<IEditCmd>&& cmd) {
    if (!m_child.empty() && m_child.back()->canCoalesceWith(cmd.get())) {
        m_child.back()->coalesceFrom(std::move(cmd));
        return;
    }

    m_child.push_back(std::move(cmd));
}

bool CompositeEditCmd::apply(IDocument& doc) {
    for (auto& it : m_child) {
        it->apply(doc);
    }
    return true;
}

bool CompositeEditCmd::undo(IDocument& doc) {
    for (size_t i = m_child.size(); i > 0; --i) {
        m_child[i - 1]->undo(doc);
    }
    return true;
}

}  // namespace cave
