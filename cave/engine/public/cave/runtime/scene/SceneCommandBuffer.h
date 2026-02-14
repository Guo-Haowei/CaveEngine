// =============================================================================
// File: public/cave/runtime/scene/SceneCommandBuffer.h
// =============================================================================
#pragma once
#include <type_traits>
#include <vector>
#include "cave/core/ids/Entity.h"
#include "cave/core/math/Vector.h"
#include "cave/core/string/FixedString.h"
#include "cave/runtime/ecs/ComponentRegistry.h"

namespace cave {

class Scene;

constexpr size_t kPropertyNameMax = 32;

class SceneCommandBuffer {
    enum class Op : uint16_t {
        CreateEntity,
        DestroyEntity,
        AddComponent,
        RemoveComponent,
        ChangeProperty,
    };

    struct Header {
        Op op;
        uint16_t size;
    };

    struct Payload_Create {
        ecs::Entity out_temp;
    };

    struct Payload_Destroy {
        ecs::Entity entity;
    };

    struct Payload_Component {
        ecs::Entity entity;
        BuildInComponentId type;
    };

    struct Payload_Property {
        ecs::Entity entity;
        BuildInComponentId type;
        FixedString<kPropertyNameMax> property_name;
        uint32_t data_size;
    };

public:
    explicit SceneCommandBuffer() = default;

    ecs::Entity Create() {
        Payload_Create e{ AllocateTempEntity() };
        WriteEntityRecord(Op::CreateEntity, &e, sizeof(e));
        return e.out_temp;
    }

    ecs::Entity Resolve(ecs::Entity p_entity) const noexcept;

    void Destroy(ecs::Entity p_entity) {
        Payload_Destroy e{ p_entity };
        WriteEntityRecord(Op::DestroyEntity, &e, sizeof(e));
    }

    void Add(ecs::Entity p_entity, BuildInComponentId p_id) {
        WriteComponentRecord(Op::AddComponent, p_entity, p_id);
    }

    void Remove(ecs::Entity p_entity, BuildInComponentId p_id) {
        WriteComponentRecord(Op::RemoveComponent, p_entity, p_id);
    }

    template<typename T>
    void SetProperty(ecs::Entity p_entity, BuildInComponentId p_id, std::string_view p_property, const T& p_value) {
        static_assert(std::is_trivially_copyable_v<T>);
        WritePropertyRecord(Op::ChangeProperty, p_entity, p_id, p_property, &p_value, sizeof(T));
    }

    // -------------------------------------------------------------------------
    // Wrappers
    // -------------------------------------------------------------------------
    void SetName(ecs::Entity p_entity, std::string_view p_value) {
        SetProperty(p_entity, NameComponent_Id, "name", FixedString<64>(p_value));
    }

    void SetScale(ecs::Entity p_entity, const math::Vector3f& p_value) {
        SetProperty(p_entity, TransformComponent_Id, "scale", p_value);
    }

    void Playback(Scene& p_scene);

private:
    static constexpr uint32_t kTmpBase = 0x80000000u;

    static bool IsTemp(ecs::Entity p_id) noexcept { return p_id.GetId() >= kTmpBase; }

    ecs::Entity AllocateTempEntity() noexcept { return ecs::Entity(m_next_entity++); }

    void SetRemap(ecs::Entity p_temp, ecs::Entity p_real);

    void WriteEntityRecord(Op p_op,
                           const void* p_payload,
                           uint16_t p_payload_size);

    void WriteComponentRecord(Op p_op,
                              ecs::Entity p_entity,
                              BuildInComponentId p_type);

    void WritePropertyRecord(Op p_op,
                             ecs::Entity p_entity,
                             BuildInComponentId p_type,
                             std::string_view p_property,
                             const void* p_data,
                             uint32_t p_data_size);

    void DispatchComponentOp(Scene& p_scene,
                             Op p_op,
                             ecs::Entity p_entity,
                             BuildInComponentId p_type_id);

    void DispatchPropertyOp(Scene& p_scene,
                            ecs::Entity p_entity,
                            const Payload_Property& p_payload,
                            const void* p_data);

    uint32_t m_next_entity = kTmpBase;
    std::vector<uint8_t> m_bytes;
    std::vector<ecs::Entity> m_remap;
};

}  // namespace cave
