// =============================================================================
// File: cave/runtime/scene/SceneTickContext.h
// =============================================================================
#pragma once
#include <cstdint>

#include "cave/core/typedefs.h"
#include "cave/core/ids/SceneId.h"

namespace cave {

enum class SceneTickDomain : uint32_t {
    Editor = 1,
    Simulate = 2,
};

DEFINE_ENUM_BITWISE_OPERATIONS(SceneTickDomain)

struct SceneTickContext {
    SceneTickDomain domain;
    SceneId scene_id;
    float dt;
};

}  // namespace cave