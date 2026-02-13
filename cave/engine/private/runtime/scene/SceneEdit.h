#pragma once
#include "cave/core/reflection/Meta.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class SceneEdit {
public:
    static bool ModifyFieldRaw(void* p_object,
                               const MetaTableFields& p_fields,
                               std::string_view p_property,
                               const void* p_data,
                               uint32_t p_data_size,
                               void* p_old_data = nullptr);

    template<ComponentType T>
    static bool ModifyField(Scene& p_scene,
                            ecs::Entity p_entity,
                            std::string_view p_property,
                            const void* p_data,
                            uint32_t p_data_size,
                            void* p_old_data = nullptr) {

        const MetaTableFields& meta_table = MetaDataTable<T>::GetFields();
        T* component = p_scene.GetComponent<T>(p_entity);
        return ModifyFieldRaw(component, meta_table, p_property, p_data, p_data_size, p_old_data);
    }
};

}  // namespace cave
