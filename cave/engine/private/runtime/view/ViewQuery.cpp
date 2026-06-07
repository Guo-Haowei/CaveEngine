#include "cave/runtime/view/ViewQuery.h"

#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

const ViewRecord* ViewQuery::Resolve(ViewId p_view_id) const {
    return m_view.Resolve(p_view_id);
}

}  // namespace cave
