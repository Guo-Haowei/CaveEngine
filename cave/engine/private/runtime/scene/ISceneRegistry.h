#pragma once
#include "cave/core/ids/SceneId.h"

#include "engine/private/runtime/framework/Module.h"

namespace cave {

class Scene;
class IApplication;

// @TODO: this doesn't have to be an interface,
// move the implementation to Impl class instead
class ISceneRegistry : public Module {

public:
    using Module::Module;

    virtual SceneId Create() = 0;

    virtual SceneId Clone(SceneId p_id) = 0;

    virtual SceneId Register(std::unique_ptr<Scene> p_scene) = 0;

    virtual void Destroy(SceneId p_id) = 0;

    virtual Scene* Resolve(SceneId p_id) = 0;

    virtual const Scene* Resolve(SceneId p_id) const = 0;

    virtual bool IsAlive(SceneId p_id) const = 0;
};

}  // namespace cave
