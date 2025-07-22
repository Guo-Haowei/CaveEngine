#pragma once
#include "engine/assets/asset_handle.h"
#include "engine/reflection/reflection.h"

namespace cave {

class Archive;

class LuaScriptComponent {
    CAVE_META(LuaScriptComponent)

    CAVE_PROP()
    std::string m_class_name;

    CAVE_PROP(editor = Asset)
    Guid m_source_id;

    // Non-Serialized
    int m_instance{ 0 };
    Handle<BlobAsset> m_source_handle;

public:
    LuaScriptComponent& SetClassName(std::string_view p_class_name);

    void SetResourceGuid(const Guid& p_guid);
    const Guid& GetResourceGuid() const { return m_source_id; }

    const std::string& GetClassName() const { return m_class_name; }
    std::string& GetClassNameRef() { return m_class_name; }

    int GetInstance() const { return m_instance; }

    void Serialize(Archive& p_archive, uint32_t p_version);
    void OnDeserialized();

private:
    friend class LuaScriptManager;
};

}  // namespace cave
