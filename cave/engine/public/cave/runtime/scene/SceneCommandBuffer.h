// =============================================================================
// File: cave/runtime/scene/SceneCommandBuffer.h
// =============================================================================
#pragma once
#include <type_traits>

#include "cave/core/containers/Containers.h"
#include "cave/core/ids/Entity.h"
#include "cave/core/math/Vec.h"
#include "cave/core/containers/FixedStack.h"
#include "cave/core/containers/FixedString.h"
#include "cave/runtime/ecs/ComponentRegistry.h"

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
    ComponentId cid;
};

struct SceneCmd_PayloadProperty {
    ecs::Entity ent;
    ComponentId cid;
    SceneCmd_PropType ptype;
    PropertyId pid;
    uint32_t data_size;
    uint32_t ele_count;
};

class SceneCommandBuffer {
public:
    explicit SceneCommandBuffer() = default;

    void reset() {
        m_next_entity = kSceneCmdTmpBase;
        m_bytes.clear();
    }

    ecs::Entity createEntity();
    void destroyEntity(ecs::Entity ent);

    void addComponent(ecs::Entity ent, ComponentId cid);
    void removeComponent(ecs::Entity ent, ComponentId cid);

    void setProperty(ecs::Entity ent,
                     ComponentId cid,
                     const PropertyId& pid,
                     const ecs::Entity& value);

    template<size_t N>
    void setProperty(ecs::Entity ent,
                     ComponentId cid,
                     const PropertyId& pid,
                     const FixedStack<ecs::Entity, N>& value) {
        writePropertyRecord(SceneCmd_Op::AssignProperty,
                            ent,
                            cid,
                            pid,
                            &value,
                            sizeof(value),
                            SceneCmd_PropType::Entity,
                            static_cast<uint32_t>(value.size()));
    }

    template<typename T>
    void setProperty(ecs::Entity ent,
                     ComponentId cid,
                     const PropertyId& pid,
                     const T& value) {
        static_assert(std::is_trivially_copyable_v<T>);
        writePropertyRecord(SceneCmd_Op::AssignProperty,
                            ent,
                            cid,
                            pid,
                            &value,
                            sizeof(value),
                            SceneCmd_PropType::PlainData);
    }

    const uint8_t* bytes() const { return m_bytes.data(); }
    const size_t byteSize() const { return m_bytes.size(); }

    uint32_t allocationCount() const noexcept { return m_next_entity - kSceneCmdTmpBase; }

private:
    ecs::Entity allocateTempEntity() noexcept { return ecs::Entity(m_next_entity++); }

    void writeEntityRecord(SceneCmd_Op op, ecs::Entity ent);

    void writeComponentRecord(SceneCmd_Op op,
                              ecs::Entity ent,
                              ComponentId cid);

    void writePropertyRecord(SceneCmd_Op op,
                             ecs::Entity ent,
                             ComponentId cid,
                             PropertyId pid,
                             const void* data,
                             uint32_t data_size,
                             SceneCmd_PropType ptype,
                             uint32_t ele_count);

    void writePropertyRecord(SceneCmd_Op op,
                             ecs::Entity ent,
                             ComponentId cid,
                             PropertyId pid,
                             const void* data,
                             uint32_t data_size,
                             SceneCmd_PropType ptype) {
        writePropertyRecord(op,
                            ent,
                            cid,
                            pid,
                            data,
                            data_size,
                            ptype,
                            1);
    }

    uint32_t m_next_entity = kSceneCmdTmpBase;
    Vector<uint8_t> m_bytes;
};

}  // namespace cave