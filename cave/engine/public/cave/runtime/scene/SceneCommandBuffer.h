// =============================================================================
// File: public/cave/runtime/scene/SceneCommandBuffer.h
// =============================================================================
#pragma once
#include <type_traits>
#include <vector>
#include "cave/core/math/Vector.h"
#include "cave/core/containers/FixedStack.h"
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/ComponentRegistry.h"
#include "cave/runtime/ecs/Entity.h"

namespace cave {

class SceneCommandExecutor;

constexpr uint32_t kSceneCmdTmpBase = 0x80000000u;

enum class SceneCmd_Op : uint8_t {
    CreateEntity,
    DestroyEntity,

    AddComponent,
    RemoveComponent,

    // @TODO: array, map?
    AssignProperty,
};

enum class SceneCmd_PropType : uint8_t {
    PlainData,
    Entity,  // Need resolve if entity
};

struct SceneCmd_Header {
    SceneCmd_Op op;
    uint8_t flags;
    uint16_t size;
};

struct SceneCmd_PayloadEntity {
    ecs::Entity ent;
};

struct SceneCmd_PayloadComponent {
    ecs::Entity ent;
    BuiltinComponentId cid;
};

struct SceneCmd_PayloadProperty {
    ecs::Entity ent;
    BuiltinComponentId cid;
    SceneCmd_PropType ptype;
    PropertyId pid;
    uint32_t data_size;
    uint32_t ele_count;
};

class SceneCommandBuffer {
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

    template<size_t N>
    void SetProperty(ecs::Entity p_ent,
                     BuiltinComponentId p_cid,
                     const PropertyId& p_pid,
                     const FixedStack<ecs::Entity, N>& p_value) {
        WritePropertyRecord(SceneCmd_Op::AssignProperty,
                            p_ent,
                            p_cid,
                            p_pid,
                            &p_value,
                            sizeof(p_value),
                            SceneCmd_PropType::Entity,
                            static_cast<uint32_t>(p_value.size()));
    }

    template<typename T>
    void SetProperty(ecs::Entity p_ent,
                     BuiltinComponentId p_cid,
                     const PropertyId& p_pid,
                     const T& p_value) {
        static_assert(std::is_trivially_copyable_v<T>);
        WritePropertyRecord(SceneCmd_Op::AssignProperty,
                            p_ent,
                            p_cid,
                            p_pid,
                            &p_value,
                            sizeof(p_value),
                            SceneCmd_PropType::PlainData);
    }

    const uint8_t* Data() const { return m_bytes.data(); }
    const size_t Size() const { return m_bytes.size(); }

    uint32_t GetAllocationCount() const noexcept { return m_next_entity - kSceneCmdTmpBase; }

private:
    ecs::Entity AllocateTempEntity() noexcept { return ecs::Entity(m_next_entity++); }

    void WriteEntityRecord(SceneCmd_Op p_op, ecs::Entity p_ent);

    void WriteComponentRecord(SceneCmd_Op p_op,
                              ecs::Entity p_ent,
                              BuiltinComponentId p_cid);

    void WritePropertyRecord(SceneCmd_Op p_op,
                             ecs::Entity p_ent,
                             BuiltinComponentId p_cid,
                             PropertyId p_pid,
                             const void* p_data,
                             uint32_t p_data_size,
                             SceneCmd_PropType p_ptype,
                             uint32_t p_ele_count);

    void WritePropertyRecord(SceneCmd_Op p_op,
                             ecs::Entity p_ent,
                             BuiltinComponentId p_cid,
                             PropertyId p_pid,
                             const void* p_data,
                             uint32_t p_data_size,
                             SceneCmd_PropType p_ptype) {
        WritePropertyRecord(p_op,
                            p_ent,
                            p_cid,
                            p_pid,
                            p_data,
                            p_data_size,
                            p_ptype,
                            1);
    }

    uint32_t m_next_entity = kSceneCmdTmpBase;
    std::vector<uint8_t> m_bytes;
};

}  // namespace cave