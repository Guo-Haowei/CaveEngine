#pragma once
#include "engine/private/runtime/scene/ISceneManager.h"

namespace cave {

class Scene;

class SceneManager : public ISceneManager {
public:
    SceneManager()
        : ISceneManager("SceneManager") {}

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;
    void Update() override;

    SceneId Create(const SceneDesc& p_desc) override;
    SceneId Register(std::unique_ptr<Scene> p_scene, const SceneDesc& p_desc) override;
    void Destroy(SceneId p_id) override;

    Scene* Resolve(SceneId p_id) override;
    const Scene* Resolve(SceneId p_id) const override;

    bool IsAlive(SceneId p_id) const override;

#if USING(DEBUG_BUILD)
    const char* GetDebugName(SceneId p_id) const override;
#endif

private:
    struct Slot {
        uint32_t gen;
        std::unique_ptr<Scene> scene;
#if USING(DEBUG_BUILD)
        char debug_name[32];
#endif

        Slot();
    };

    SceneId Alloc();
    void Free(const SceneId& p_id);

    std::vector<Slot> m_slots;
    std::vector<uint32_t> m_free;

public:

    // @TODO: deprecated below

    std::shared_ptr<Scene> GetActiveScene() const override;

    void BumpRevision() override { ++m_revision; }

protected:
    bool TrySwapScene();

    uint32_t m_revision = 0;
    uint32_t m_lastRevision = 0;

    struct LoadSceneTask {
        bool replace;
        Scene* scene;
    };
};

}  // namespace cave
