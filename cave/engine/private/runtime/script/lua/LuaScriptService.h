#pragma once
#include "cave/core/ids/Guid.h"
#include "engine/private/runtime/framework/IScriptService.h"

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

class LuaScriptService : public IScriptService {

public:
    LuaScriptService()
        : IScriptService("LuaScriptService") {}

    void Update(Scene& p_scene, float p_timestep) override;
    void OnCollision(Scene& p_scene, ecs::Entity p_entity_1, ecs::Entity p_entity_2) override;

    void OnSimBegin(Scene& p_scene) override;
    void OnSimEnd() override;

protected:
    auto InitializeImpl() -> Result<void> final;
    void FinalizeImpl() final;

    ObjectFunctions FindOrAdd(lua_State* L, const Guid& p_guid, const char* p_class_name);
    Result<void> LoadMetaTable(lua_State* L, const Guid& p_guid, const char* p_class_name, ObjectFunctions& p_meta);

    std::map<Guid, ObjectFunctions> m_objects_meta;
    lua_State* m_state{ nullptr };
};

}  // namespace cave
