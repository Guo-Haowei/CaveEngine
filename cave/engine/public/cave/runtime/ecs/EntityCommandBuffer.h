// =============================================================================
// File: public/cave/runtime/ecs/EntityCommandBuffer.h
// =============================================================================
#pragma once
#include <type_traits>
#include <vector>
#include "cave/core/ids/Entity.h"
#include "cave/runtime/ecs/ComponentRegistry.h"

namespace cave {

class Scene;

class EntityCommandBuffer {
    enum class Op : uint8_t {
        CreateEntity,
        DestroyEntity,
        AddComponent,
        RemoveComponent,
        SetComponent,
    };

    struct Header {
        Op op;
        uint8_t flags;
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
        ComponentId type;
        uint32_t data_size;
    };

public:
    explicit EntityCommandBuffer() = default;

    ecs::Entity Create() {
        Payload_Create e{ AllocateTempEntity() };
        WriteRecord(Op::CreateEntity, &e, sizeof(e));
        return e.out_temp;
    }

    ecs::Entity Resolve(ecs::Entity p_entity) const noexcept;

    void Destroy(ecs::Entity p_entity) {
        Payload_Destroy e{ p_entity };
        WriteRecord(Op::DestroyEntity, &e, sizeof(e));
    }

    template<typename T>
    void Add(ecs::Entity p_entity, ComponentId p_id, const T& p_component) {
        static_assert(std::is_trivially_copyable_v<T>);
        WriteComponentRecord(Op::AddComponent, p_entity, p_id, &p_component, sizeof(T));
    }

    template<typename T>
    void Set(ecs::Entity p_entity, ComponentId p_id, const T& p_component) {
        static_assert(std::is_trivially_copyable_v<T>);
        WriteComponentRecord(Op::SetComponent, p_entity, p_id, &p_component, sizeof(T));
    }

    template<typename T>
    void Remove(ecs::Entity p_entity, ComponentId p_id, const T& p_component) {
        static_assert(std::is_trivially_copyable_v<T>);
        WriteComponentRecord(Op::RemoveComponent, p_entity, p_id, nullptr, 0);
    }

    void Playback(Scene& p_scene);

private:
    static constexpr uint32_t kTmpBase = 0x80000000u;

    static bool IsTemp(ecs::Entity p_id) noexcept { return p_id.GetId() >= kTmpBase; }

    ecs::Entity AllocateTempEntity() noexcept { return ecs::Entity(m_next_entity++); }

    void SetRemap(ecs::Entity p_temp, ecs::Entity p_real);

    void WriteRecord(Op p_op,
                     const void* p_payload,
                     uint16_t p_payload_size);

    void WriteComponentRecord(Op p_op,
                              ecs::Entity p_entity,
                              ComponentId p_type,
                              const void* p_data,
                              uint32_t p_data_size);

    void DispatchComponentOp(Scene& p_scene,
                             Op p_op,
                             ecs::Entity p_entity,
                             ComponentId p_type_id,
                             const void* p_data,
                             uint32_t p_size);

    uint32_t m_next_entity = kTmpBase;
    std::vector<uint8_t> m_bytes;
    std::vector<ecs::Entity> m_remap;
};

}  // namespace cave
