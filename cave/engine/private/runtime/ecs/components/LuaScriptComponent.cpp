#include "cave/runtime/script/lua/LuaScriptComponent.h"

#include "engine/private/core/io/archive.h"
#include "engine/private/runtime/assets/BlobAsset.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

namespace cave {

bool LuaScriptComponent::SetResourceGuid(const Guid& guid) {
    return AssetHandle::replaceGuidAndHandle(AssetType::Blob,
                                             guid,
                                             m_source_id,
                                             m_source_handle.rawHandle());
}

LuaScriptComponent& LuaScriptComponent::SetClassName(std::string_view class_name) {
    if (DEV_VERIFY(!class_name.empty())) {
        m_class_name = class_name;
    }

    return *this;
}

void LuaScriptComponent::onDeserialized() {
    auto res = AssetRegistry::singleton().findByGuid<BlobAsset>(m_source_id);
    m_source_handle = std::move(res.unwrap());
}

}  // namespace cave
