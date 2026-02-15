// =============================================================================
// File: public/cave/runtime/scene/SceneCommandBuffer.h
// =============================================================================
#pragma once
#include <type_traits>
#include <vector>
#include "cave/core/math/Vector.h"
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class SceneMutator;

class SceneCommandBuffer {
    enum class Op : uint16_t {
        CreateEntity,
        DestroyEntity,
        AddComponent,
        RemoveComponent,
        ChangeProperty,

        // high level commands
        AttachRoot,
    };

    struct Header {
        Op op;
        uint16_t size;
    };

    struct Payload_Create {
        ecs::Entity out_temp;
    };

    struct Payload_Destroy {
        ecs::Entity ent;
    };

    struct Payload_Component {
        ecs::Entity ent;
        BuildInComponentId type;
    };

    struct Payload_Property {
        ecs::Entity ent;
        BuildInComponentId type;
        PropertyId prop_id;
        uint32_t data_size;
    };

    struct Payload_AttachRoot {
        ecs::Entity ent;
    };

public:
    explicit SceneCommandBuffer() = default;

    ecs::Entity Create() {
        Payload_Create e{ AllocateTempEntity() };
        WriteEntityRecord(Op::CreateEntity, &e, sizeof(e));
        return e.out_temp;
    }

    ecs::Entity Resolve(ecs::Entity p_ent) const noexcept;

    void Destroy(ecs::Entity p_ent) {
        WriteEntityRecord(Op::DestroyEntity, &p_ent, sizeof(p_ent));
    }

    void Add(ecs::Entity p_ent, BuildInComponentId p_id) {
        WriteComponentRecord(Op::AddComponent, p_ent, p_id);
    }

    void Remove(ecs::Entity p_ent, BuildInComponentId p_id) {
        WriteComponentRecord(Op::RemoveComponent, p_ent, p_id);
    }

    void AttachRoot(ecs::Entity p_ent) {
        WriteEntityRecord(Op::AttachRoot, &p_ent, sizeof(p_ent));
    }

    template<typename T>
    void SetProperty(ecs::Entity p_ent, BuildInComponentId p_id, const PropertyId& p_prop_id, const T& p_value) {
        static_assert(std::is_trivially_copyable_v<T>);
        WritePropertyRecord(Op::ChangeProperty, p_ent, p_id, p_prop_id, &p_value, sizeof(T));
    }

    void Playback(SceneMutator& p_mut);

    bool Empty() const { return m_bytes.empty(); }

private:
    static constexpr uint32_t kTmpBase = 0x80000000u;

    static bool IsTemp(ecs::Entity p_id) noexcept { return p_id.GetId() >= kTmpBase; }

    ecs::Entity AllocateTempEntity() noexcept { return ecs::Entity(m_next_entity++); }

    void SetRemap(ecs::Entity p_temp, ecs::Entity p_real);

    void WriteEntityRecord(Op p_op,
                           const void* p_payload,
                           uint16_t p_payload_size);

    void WriteComponentRecord(Op p_op,
                              ecs::Entity p_ent,
                              BuildInComponentId p_type);

    void WritePropertyRecord(Op p_op,
                             ecs::Entity p_ent,
                             BuildInComponentId p_type,
                             PropertyId p_prop_id,
                             const void* p_data,
                             uint32_t p_data_size);

    uint32_t m_next_entity = kTmpBase;
    std::vector<uint8_t> m_bytes;
    std::vector<ecs::Entity> m_remap;
};

}  // namespace cave