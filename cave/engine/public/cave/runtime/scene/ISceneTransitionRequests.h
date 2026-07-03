// =============================================================================
// File: cave/runtime/scene/ISceneTransitionRequests.h
// =============================================================================
#pragma once
#include <string>

#include "cave/core/Option.h"
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/scene/SceneTickContext.h"

namespace cave {

class ISceneTransitionRequests {
public:
    virtual ~ISceneTransitionRequests() = default;

    virtual void requestSceneChange(std::string scene) = 0;

    virtual DebugId debugId() const = 0;
};

}  // namespace cave
