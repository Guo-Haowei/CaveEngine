#pragma once
#include "cave/core/ids/ViewId.h"
#include "cave/core/math/Box.h"

namespace cave {

struct ViewRecord {
    ViewId view_id;

    std::string debug_name;

    math::FloatRect rect{};  // view rect in window space

    //// Last render target dimensions / region.
    // math::IntRect viewport_px{};

    //// Optional: last submitted output.
    // GpuTextureId output{};

    uint64_t last_submitted_frame = 0;
    uint64_t last_visible_frame = 0;
};

}  // namespace cave
