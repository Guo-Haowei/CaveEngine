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

enum class SceneCommandOp : uint8_t {
    CreateEntity,
    DestroyEntity,
    AddComponent,
    RemoveComponent,
    ChangeProperty,
};

class SceneCommandBuffer {
    enum class PropType : uint8_t {
        PlainData,
        Entity,
        EntityArray,
    };

    struct Header;
    struct Payload_Entity;
    struct Payload_Component;
    struct Payload_Property;

public:
    explicit SceneCommandBuffer() = default;

    ecs::Entity CreateEntity();

    void DestroyEntity(ecs::Entity p_ent);

    void AddComponent(ecs::Entity p_ent, BuiltinComponentId p_id);

    void RemoveComponent(ecs::Entity p_ent, BuiltinComponentId p_id);

    void SetProperty(ecs::Entity p_ent,
                     BuiltinComponentId p_cid,
                     const PropertyId& p_pid,
                     const ecs::Entity& p_value);

    template<typename T>
    void SetProperty(ecs::Entity p_ent,
                     BuiltinComponentId p_cid,
                     const PropertyId& p_pid,
                     const T& p_value) {
        static_assert(std::is_trivially_copyable_v<T>);
        WritePropertyRecord(SceneCommandOp::ChangeProperty,
                            p_ent,
                            p_cid,
                            p_pid,
                            &p_value,
                            sizeof(T),
                            PropType::PlainData);
    }

    ecs::Entity Resolve(ecs::Entity p_ent) const noexcept;

    void Playback(SceneMutator& p_mut);

    bool Empty() const { return m_bytes.empty(); }

private:
    static constexpr uint32_t kTmpBase = 0x80000000u;

    static bool IsTemp(ecs::Entity p_id) noexcept { return p_id.GetId() >= kTmpBase; }

    ecs::Entity AllocateTempEntity() noexcept { return ecs::Entity(m_next_entity++); }

    void SetRemap(ecs::Entity p_temp, ecs::Entity p_real);

    void WriteEntityRecord(SceneCommandOp p_op, ecs::Entity p_ent);

    void WriteComponentRecord(SceneCommandOp p_op,
                              ecs::Entity p_ent,
                              BuiltinComponentId p_cid);

    void WritePropertyRecord(SceneCommandOp p_op,
                             ecs::Entity p_ent,
                             BuiltinComponentId p_cid,
                             PropertyId p_pid,
                             const void* p_data,
                             uint32_t p_data_size,
                             PropType p_ptype);

    uint32_t m_next_entity = kTmpBase;
    std::vector<uint8_t> m_bytes;
    std::vector<ecs::Entity> m_remap;
};

}  // namespace cave