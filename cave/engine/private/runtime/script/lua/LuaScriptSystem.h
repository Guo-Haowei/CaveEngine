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

    void update(float dt) override;

    DebugId debugId() const override { return debug_id_; }

protected:
    void onAttach() override;
    void onDetach() override;

    ObjectFunctions findOrAdd(lua_State* L,
                              const Guid& guid,
                              const char* class_name);

    Result<void> loadMetaTable(lua_State* L,
                               const Guid& guid,
                               const char* class_name,
                               ObjectFunctions& meta);

    const DebugId debug_id_;
    std::map<Guid, ObjectFunctions> meta_lookup_;
    lua_State* state_{ nullptr };
};

}  // namespace cave
