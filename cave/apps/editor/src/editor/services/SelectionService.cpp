#include "SelectionService.h"

namespace cave {

void SelectionService::setSelection(DocId doc_id, const SelectionKey& key) {
    m_selections[doc_id] = key;
}

SelectionKey SelectionService::primary(DocId doc_id) {
    if (auto it = m_selections.find(doc_id); it != m_selections.end()) {
        return it->second;
    }

    return {};
}

}  // namespace cave
