#pragma once
#include "cave/core/ids/Guid.h"
#include "cave/runtime/scene/ISceneSystem.h"

struct lua_State;

namespace cave {

class Scene;

struct ObjectFunctions {
    int funcNew{ 0 };
};

struct LuaMethodRefs {
    int awake_fn = 0;
    int tick_fn = 0;
};

struct LuaClassMeta {
    int class_table_ref = 0;
    int new_fn = 0;
    LuaMethodRefs methods{};
};

class LuaScriptSystem final : public ISceneSystem {
    CAVE_SCENE_SYSTEM(SceneSystemId::LuaScript)

public:
    LuaScriptSystem();
    ~LuaScriptSystem() override { clear(); }

private:
    void clear();

    void update(SceneTickContext& ctx) override;
    DebugId debugId() const override { return m_debug_id; }

    void start(SceneContext& ctx) override;

    SceneTickDomain domain() const override { return SceneTickDomain::Simulate; }

    ObjectFunctions findOrAdd(SceneContext& ctx,
                              lua_State* L,
                              const Guid& guid,
                              const char* class_name);

    Result<void> loadMetaTable(SceneContext& ctx,
                               lua_State* L,
                               const Guid& guid,
                               const char* class_name,
                               ObjectFunctions& meta);

    const DebugId m_debug_id;
    std::map<Guid, ObjectFunctions> m_meta_lookup;
    lua_State* m_state{ nullptr };
};

}  // namespace cave
