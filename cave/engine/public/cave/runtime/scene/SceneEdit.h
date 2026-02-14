#pragma once
#include "cave/core/reflection/Meta.h"
#include "cave/runtime/scene/SceneChangeEvent.h"

namespace cave {

class Scene;

// @TODO: emit events

class SceneEdit {
public:
    explicit SceneEdit(Scene& p_scene) noexcept
        : m_scene(p_scene) {
    }

    ecs::Entity CreateEntity();
    void DestroyEntity(ecs::Entity p_ent);

    void AttachChild(ecs::Entity p_child, ecs::Entity p_parent);
    void AttachChild(ecs::Entity p_child);

    template<typename T>
    bool ModifyField(ecs::Entity p_ent,
                     std::string_view p_property,
                     const void* p_data,
                     uint32_t p_data_size,
                     void* p_old_data = nullptr);

private:
    bool ModifyFieldRaw(void* p_object,
                        const MetaTableFields& p_fields,
                        std::string_view p_property,
                        const void* p_data,
                        uint32_t p_data_size,
                        void* p_old_data);

    Scene& m_scene;
};

}  // namespace cave
