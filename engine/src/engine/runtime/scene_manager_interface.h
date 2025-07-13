#pragma once
#include "engine/core/base/concurrent_queue.h"
// #include "engine/core/base/singleton.h"
#include "engine/runtime/module.h"

namespace my {

class Scene;
class Application;

class ISceneManager : public Module,
                      public ModuleCreateRegistry<ISceneManager> {
public:
    ISceneManager(std::string_view p_name)
        : Module(p_name) {}

    virtual Scene* GetActiveScene() = 0;
    virtual void Update() = 0;

#if 0
    Result<void> LoadScene(const std::string& path);
    Result<void> LoadSceneAsync(const std::string& path);

    // Save current scene
    Result<void> SaveScene(const std::string& path);

    // Unload current scene
    void UnloadScene();

    // Reload current scene from file
    Result<void> ReloadScene();
#endif
};

}  // namespace my
