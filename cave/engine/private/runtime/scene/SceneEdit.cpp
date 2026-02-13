#include "SceneEdit.h"

namespace cave {

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
