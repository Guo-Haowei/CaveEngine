#pragma once
#include "cave/core/ids/SceneId.h"

#include "engine/private/runtime/framework/Module.h"

namespace cave {

class Scene;
class IApplication;

struct SceneDesc {
    std::string debug_name;
};

class ISceneRegistry : public Module {
public:
    using Module::Module;

    SceneId Create(SceneDesc p_desc);

    SceneId Register(SceneDesc p_desc, std::unique_ptr<Scene> p_scene);

    SceneId Clone(SceneDesc p_desc, SceneId p_id);

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
