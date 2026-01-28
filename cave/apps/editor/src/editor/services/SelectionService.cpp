#include "SelectionService.h"

namespace cave {

void SelectionService::Set(DocId p_doc_id, const SelectionKey& p_key) {
    m_selections[p_doc_id] = p_key;
}

SelectionKey SelectionService::Primary(DocId p_doc_id) {
    if (auto it = m_selections.find(p_doc_id); it != m_selections.end()) {
        return it->second;
    }

    return {};
}

}  // namespace cave
