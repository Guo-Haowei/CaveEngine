#pragma once
#include "cave/core/ids/SceneId.h"
#include "cave/runtime/framework/IService.h"

namespace cave {

class Scene;
class IApplication;

class SceneRegistry : public IService {
public:
    SceneRegistry();

    SceneId Create(std::string p_name);

    SceneId Register(std::unique_ptr<Scene> p_scene);

    SceneId Clone(SceneId p_id);

    void Destroy(SceneId p_id);

    Scene* Resolve(SceneId p_id);

    const Scene* Resolve(SceneId p_id) const;

    bool IsAlive(SceneId p_id) const;

protected:
    auto InitializeImpl() -> Result<void>;
    void FinalizeImpl();

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

}  // namespace cave
