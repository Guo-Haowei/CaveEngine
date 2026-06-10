#pragma once
#include "cave/core/ids/SceneId.h"

namespace cave {

class Scene;
class IApplication;

class SceneRegistry {
public:
    SceneRegistry();
    ~SceneRegistry();

    SceneId Create(std::string name);

    SceneId Register(std::unique_ptr<Scene> scene);

    SceneId Clone(SceneId sid);

    void Destroy(SceneId sid);

    Scene* Resolve(SceneId sid);

    const Scene* Resolve(SceneId sid) const;

    bool IsAlive(SceneId sid) const;

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

}  // namespace cave
