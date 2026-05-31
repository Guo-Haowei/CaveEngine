#pragma once
#include "cave/core/ids/ViewId.h"

namespace cave {

struct ViewRecord {
    ViewId view_id;

    // std::string debug_name;

    //// Where the view is displayed in application/window space.
    //// Used for mouse -> view-local conversion.
    // math::FloatRect screen_rect{};

    //// Last render target dimensions / region.
    // math::IntRect viewport_px{};

    //// Optional: last submitted output.
    // GpuTextureId output{};

    uint64_t last_submitted_frame = 0;
    uint64_t last_visible_frame = 0;
};

}  // namespace cave
