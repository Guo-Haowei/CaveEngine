// =============================================================================
// File: public/cave/runtime/scene/SceneId.h
// =============================================================================
#pragma once
#include <cstdint>
#include "cave/runtime/core/GenId.h"

namespace cave {

class Scene;

using SceneId = GenId<Scene>;

}  // namespace cave
