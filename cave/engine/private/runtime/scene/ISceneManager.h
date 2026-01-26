#pragma once
#include "cave/runtime/core/Singleton.h"
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/runtime/framework/Module.h"

namespace cave {

class Scene;
class IApplication;

struct SceneDesc {
#if USING(DEBUG_BUILD)
    std::string_view debug_name;
#endif
};

class ISceneManager : public Module,
                      public ModuleCreateRegistry<ISceneManager>,
                      public Singleton<ISceneManager> {
public:
    ISceneManager(std::string_view p_name)
        : Module(p_name) {}

    virtual SceneId Create(const SceneDesc& p_desc) = 0;

    virtual SceneId Register(std::unique_ptr<Scene> p_scene,
                             const SceneDesc& p_desc) = 0;

    virtual void Destroy(SceneId p_id) = 0;

    virtual Scene* Resolve(SceneId p_id) = 0;
    virtual const Scene* Resolve(SceneId p_id) const = 0;

    virtual bool IsAlive(SceneId p_id) const = 0;

#if USING(DEBUG_BUILD)
    virtual const char* GetDebugName(SceneId p_id) const = 0;
#endif
};

}  // namespace cave
