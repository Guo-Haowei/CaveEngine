#pragma once
#include "cave/runtime/scene/SceneId.h"

#include "engine/private/runtime/framework/Module.h"

namespace cave {

class Scene;
class IApplication;

class SceneManager : public Module,
                     public ModuleCreateRegistry<SceneManager> {
    struct Slot {
        uint32_t gen;
        std::unique_ptr<Scene> scene;
    };

public:
    SceneManager();
    ~SceneManager();

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    SceneId Create();

    SceneId Clone(SceneId p_id);

    SceneId Register(std::unique_ptr<Scene> p_scene);

    void Destroy(SceneId p_id);

    Scene* Resolve(SceneId p_id);
    const Scene* Resolve(SceneId p_id) const;

    bool IsAlive(SceneId p_id) const;

private:
    SceneId Alloc();
    void Free(SceneId p_id);

    std::vector<Slot> m_slots;
    std::vector<uint32_t> m_free;
};

}  // namespace cave
