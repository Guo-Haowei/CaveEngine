#pragma once
#include "engine/core/base/concurrent_queue.h"
#include "engine/core/base/singleton.h"
#include "engine/runtime/module.h"

namespace cave {

class Scene;
class Application;

class ISceneManager : public Module,
                      public ModuleCreateRegistry<ISceneManager>,
                      public Singleton<ISceneManager> {
public:
    ISceneManager(std::string_view p_name)
        : Module(p_name) {}

    virtual void SetActiveScene(Scene* p_scene) = 0;
    virtual Scene* GetActiveScene() const = 0;

    virtual void Update() = 0;
    virtual void BumpRevision() = 0;
};

}  // namespace cave
