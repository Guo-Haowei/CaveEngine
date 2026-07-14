// =============================================================================
// File: cave/runtime/script/lua/LuaScriptComponent.h
// =============================================================================
#pragma once
#include "cave/core/ids/Guid.h"
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/assets/AssetHandle.h"
#include "cave/runtime/ecs/ComponentDefines.h"

namespace cave {

class LuaScriptComponent {
    CAVE_COMPONENT(LuaScriptComponent)

private:
    CAVE_PROP()
    FixedString<32> m_class_name;

    CAVE_PROP(editor = Asset)
    Guid m_source_id;

    // Non-Serialized
    int m_instance{ 0 };
    Handle<BlobAsset> m_source_handle;

public:
    LuaScriptComponent& SetClassName(std::string_view p_class_name);

    bool SetResourceGuid(const Guid& p_guid);
    const Guid& GetResourceGuid() const { return m_source_id; }

    std::string_view GetClassName() const { return m_class_name.view(); }
    auto& GetClassNameRef() { return m_class_name; }

    int GetInstance() const { return m_instance; }

    void onDeserialized();

private:
    friend class LuaScriptSystem;
};

}  // namespace cave
