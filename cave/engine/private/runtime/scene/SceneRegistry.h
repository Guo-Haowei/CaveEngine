#include "ISceneRegistry.h"

#include "engine/private/core/GenIdRegistry.h"

namespace cave {

class Scene;
struct CommandArgs;
struct CommandContext;

class SceneRegistry : public ISceneRegistry,
                      protected GenIdRegistry<Scene> {
    using Base = GenIdRegistry<Scene>;

public:
    SceneRegistry();

    auto InitializeImpl() -> Result<void> override;
    void FinalizeImpl() override;

    SceneId Create() override;

    SceneId Clone(SceneId p_id) override;

    SceneId Register(std::unique_ptr<Scene> p_scene) override;

    void Destroy(SceneId p_id) override;

    Scene* Resolve(SceneId p_id) override {
        return Base::Resolve(p_id);
    }

    const Scene* Resolve(SceneId p_id) const override {
        return Base::Resolve(p_id);
    }

    bool IsAlive(SceneId p_id) const override {
        return Base::IsAlive(p_id);
    }

private:
    bool Dump_Cmd(CommandContext& p_ctx, const CommandArgs& p_args);
};

}  // namespace cave
