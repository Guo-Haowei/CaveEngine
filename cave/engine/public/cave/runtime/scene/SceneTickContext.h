// =============================================================================
// File: cave/runtime/scene/SceneTickContext.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/runtime/scene/SceneContext.h"

namespace cave {

enum class SceneTickMode : uint8_t {
    Editor,
    Simulation,
};

struct SceneTickContext {
    SceneTickMode mode;
    float dt;
    SceneContext& scene_ctx;
};

}  // namespace cave