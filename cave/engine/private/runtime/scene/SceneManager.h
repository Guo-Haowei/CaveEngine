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

class SceneManager : public Module,
                     public ModuleCreateRegistry<SceneManager>,
                     public Singleton<SceneManager> {
    struct Slot {
        uint32_t gen;
        std::unique_ptr<Scene> scene;
#if USING(DEBUG_BUILD)
        char debug_name[32];
#endif
        Slot();
    };

public:
    SceneManager();
    ~SceneManager();

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    SceneId Create(const SceneDesc& p_desc);

    SceneId Clone(const SceneDesc& p_desc, SceneId p_id);

    SceneId Register(const SceneDesc& p_desc, std::unique_ptr<Scene> p_scene);

    void Destroy(SceneId p_id);

    Scene* Resolve(SceneId p_id);
    const Scene* Resolve(SceneId p_id) const;

    bool IsAlive(SceneId p_id) const;

#if USING(DEBUG_BUILD)
    const char* GetDebugName(SceneId p_id) const;
#endif

private:
    SceneId Alloc();
    void Free(SceneId p_id);

    std::vector<Slot> m_slots;
    std::vector<uint32_t> m_free;
};

}  // namespace cave
