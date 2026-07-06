#pragma once
#include "cave/core/diagnostics/Command.h"
#include "cave/core/ids/SceneId.h"

namespace cave {

class Scene;
class IApplication;

enum class SceneSource : uint8_t {
    Asset,
    Editor,
    Runtime,
    Thumbnail,
};

struct SceneDesc {
    SceneSource source;
    std::string debug_name;
};

class SceneRegistry {
public:
    SceneRegistry();
    ~SceneRegistry();

    SceneId createScene(SceneDesc desc);
    SceneId registerScene(SceneDesc desc, std::unique_ptr<Scene>&& scene);

    bool replaceScene(SceneId id, std::unique_ptr<Scene>&& scene);

    SceneId cloneScene(SceneDesc desc, SceneId scene_id);
    SceneId cloneScene(SceneDesc desc, const Scene& scene);

    void destroyScene(SceneId scene_id);

    Scene* resolve(SceneId scene_id);

    const Scene* resolve(SceneId scene_id) const;

    bool isAlive(SceneId scene_id) const;

#if USING(USE_COMMAND)
    bool Cmd_dump(CommandContext& ctx, const CommandArgs& args);
#endif

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

}  // namespace cave
