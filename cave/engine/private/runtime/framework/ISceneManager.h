#pragma once
#include "engine/private/core/base/concurrent_queue.h"
#include "engine/private/core/base/singleton.h"
#include "engine/private/runtime/framework/Module.h"

namespace cave {

class Scene;
class Application;

class ISceneManager : public Module,
                      public ModuleCreateRegistry<ISceneManager>,
                      public Singleton<ISceneManager> {
public:
    ISceneManager(std::string_view p_name)
        : Module(p_name) {}

    virtual std::shared_ptr<Scene> GetActiveScene() const = 0;

    virtual void Update() = 0;
    virtual void BumpRevision() = 0;

    // @TODO: get rid of these
    virtual void OpenSimScene(const std::shared_ptr<Scene>&) {};
    virtual void CloseSimScene() {}
};

}  // namespace cave
