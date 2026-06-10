#pragma once
#include "cave/core/diagnostics/Command.h"
#include "cave/core/ids/SceneId.h"

namespace cave {

class Scene;
class IApplication;

class SceneRegistry {
public:
    SceneRegistry();
    ~SceneRegistry();

    SceneId createScene(std::string name);

    SceneId registerScene(std::unique_ptr<Scene> scene);

    SceneId cloneScene(SceneId scene_id);

    void destroyScene(SceneId scene_id);

    Scene* resolve(SceneId scene_id);

    const Scene* resolve(SceneId scene_id) const;

    bool isAlive(SceneId scene_id) const;

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

private:
    class Impl;

    std::unique_ptr<Impl> impl_;
};

}  // namespace cave
