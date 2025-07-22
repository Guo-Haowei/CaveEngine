#include "lua_script_component.h"

#include "engine/assets/blob_asset.h"
#include "engine/core/io/archive.h"
#include "engine/runtime/asset_registry.h"

namespace cave {

void LuaScriptComponent::SetResourceGuid(const Guid& p_guid) {
    AssetHandle::ReplaceGuidAndHandle(AssetType::Blob,
                                      p_guid,
                                      m_source_id,
                                      m_source_handle.RawHandle());
}

LuaScriptComponent& LuaScriptComponent::SetClassName(std::string_view p_class_name) {
    if (DEV_VERIFY(!p_class_name.empty())) {
        m_class_name = p_class_name;
    }

    return *this;
}

void LuaScriptComponent::Serialize(Archive& p_archive, uint32_t) {
    p_archive.ArchiveValue(m_source_id);
    p_archive.ArchiveValue(m_class_name);
}

void LuaScriptComponent::OnDeserialized() {
    auto res = AssetRegistry::GetSingleton().FindByGuid<BlobAsset>(m_source_id);
    m_source_handle = std::move(res.unwrap());
}

}  // namespace cave
