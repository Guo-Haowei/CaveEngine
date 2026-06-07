#include "cave/runtime/view/ViewQuery.h"

#include "engine/private/runtime/view/ViewManager.h"

namespace cave {

const ViewRecord* ViewQuery::resolve(ViewId view_id) const {
    return view_.Resolve(view_id);
}

}  // namespace cave
