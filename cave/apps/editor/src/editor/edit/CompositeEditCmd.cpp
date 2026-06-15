#include "CompositeEditCmd.h"

namespace cave {

void CompositeEditCmd::AddCommand(std::unique_ptr<IEditCmd>&& p_cmd) {
    if (!m_child.empty() && m_child.back()->canCoalesceWith(p_cmd.get())) {
        m_child.back()->coalesceFrom(std::move(p_cmd));
        return;
    }

    m_child.push_back(std::move(p_cmd));
}

bool CompositeEditCmd::apply(IDocument& p_doc) {
    for (auto& it : m_child) {
        it->apply(p_doc);
    }
    return true;
}

bool CompositeEditCmd::undo(IDocument& p_doc) {
    for (size_t i = m_child.size(); i > 0; --i) {
        m_child[i - 1]->undo(p_doc);
    }
    return true;
}

}  // namespace cave
