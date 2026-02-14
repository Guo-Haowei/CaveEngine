#include "cave/runtime/scene/SceneEdit.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

#define MODIFY_FIELD_IMPL(T)                                                                                                                     \
    template<>                                                                                                                                   \
    bool SceneEdit::ModifyField<T>(ecs::Entity p_ent, std::string_view p_property, const void* p_data, uint32_t p_data_size, void* p_old_data) { \
        const MetaTableFields& meta_table = MetaDataTable<T>::GetFields();                                                                       \
        T* component = m_scene.GetComponent<T>(p_ent);                                                                                           \
        return ModifyFieldRaw(component, meta_table, p_property, p_data, p_data_size, p_old_data);                                               \
    }

#define REGISTER_COMPONENT(T, ...) MODIFY_FIELD_IMPL(T)
REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT

bool SceneEdit::ModifyFieldRaw(void* p_object,
                               const MetaTableFields& p_fields,
                               std::string_view p_property,
                               const void* p_data,
                               uint32_t p_data_size,
                               void* p_old_data) {
    if (!p_object) {
        return false;
    }

    for (const FieldMetaBase* field : p_fields) {
        if (p_property == field->name) {
            char* data = reinterpret_cast<char*>(p_object) + field->offset;
            if (p_old_data) {
                std::memcpy(p_old_data, data, p_data_size);
            }
            std::memcpy(data, p_data, p_data_size);
            return true;
        }
    }

    LOG_ERROR("SceneEdit::ModifyFieldRaw: field '{}' not found");
    return false;
}

}  // namespace cave
