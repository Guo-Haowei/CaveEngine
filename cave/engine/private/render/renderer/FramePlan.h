#pragma once
#include "engine/private/runtime/view/ResolvedView.h"
// @TODO: fix
#include "engine/private/render/renderer/FrameData.h"

namespace cave::render {

struct FramePlan {
    Vector<ResolvedView> views;
    Vector<FrameData> frame_data;
};

}  // namespace cave::render
