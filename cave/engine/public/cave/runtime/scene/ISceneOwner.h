// =============================================================================
// File: cave/runtime/scene/ISceneOwner.h
// =============================================================================
#pragma once
#include <string>

#include "cave/core/Option.h"
#include "cave/core/ids/DebugId.h"
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/scene/SceneTickContext.h"

namespace cave {

class ISceneOwner;

struct SceneTickRequest {
    SceneTickMode mode;
    SceneId scene_id;
    ISceneOwner& owner;
};

class ISceneOwner {
public:
    virtual ~ISceneOwner() = default;

    virtual void collectSceneTicks(std::vector<SceneTickRequest>& out_requests) = 0;
    virtual void commitSceneChange() = 0;

    virtual void requestSceneChange(std::string scene) {
        pending_change_ = Some(std::move(scene));
    }

    virtual DebugId debugId() const = 0;

protected:
    Option<std::string> pending_change_;
};

}  // namespace cave
