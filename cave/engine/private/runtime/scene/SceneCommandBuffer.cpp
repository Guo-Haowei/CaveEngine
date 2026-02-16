#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneCommandExecutor.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;

Entity SceneCommandBuffer::CreateEntity() {
    Entity e = AllocateTempEntity();
    WriteEntityRecord(SceneCmd_Op::CreateEntity, e);
    return e;
}

void SceneCommandBuffer::DestroyEntity(Entity p_ent) {
    WriteEntityRecord(SceneCmd_Op::DestroyEntity, p_ent);
}

void SceneCommandBuffer::AddComponent(Entity p_ent, BuiltinComponentId p_id) {
    WriteComponentRecord(SceneCmd_Op::AddComponent, p_ent, p_id);
}

void SceneCommandBuffer::RemoveComponent(Entity p_ent, BuiltinComponentId p_id) {
    WriteComponentRecord(SceneCmd_Op::RemoveComponent, p_ent, p_id);
}

void SceneCommandBuffer::SetProperty(Entity p_ent,
                                     BuiltinComponentId p_cid,
                                     const PropertyId& p_pid,
                                     const Entity& p_value) {
    WritePropertyRecord(SceneCmd_Op::AssignProperty,
                        p_ent,
                        p_cid,
                        p_pid,
                        &p_value,
                        sizeof(Entity),
                        SceneCmd_PropType::Entity);
}

Entity SceneCommandBuffer::Resolve(Entity p_ent) const noexcept {
    if (!IsTemp(p_ent)) return p_ent;

    const uint32_t index = p_ent.GetId() - kTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    return Entity::Null();
}

void SceneCommandBuffer::Playback(SceneCommandExecutor& p_mut) {
    m_remap.clear();
    m_remap.resize(m_next_entity - kTmpBase, Entity::Null());

    const uint8_t* p = m_bytes.data();
    const uint8_t* end = m_bytes.data() + m_bytes.size();

    while (p < end) {
        const SceneCmd_Header* header = reinterpret_cast<const SceneCmd_Header*>(p);
        const uint8_t* payload_raw = reinterpret_cast<const uint8_t*>(header + 1);

        switch (header->op) {
            case SceneCmd_Op::CreateEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                Entity real = p_mut.CreateEntity();
                SetRemap(e, real);
            } break;
            case SceneCmd_Op::DestroyEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                p_mut.RemoveEntity(Resolve(e));
            } break;
            case SceneCmd_Op::AddComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                p_mut.AddComponent(Resolve(payload->ent), payload->cid);
            } break;
            case SceneCmd_Op::RemoveComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                p_mut.RemoveComponent(Resolve(payload->ent), payload->cid);
            } break;
            case SceneCmd_Op::AssignProperty: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadProperty*>(payload_raw);
                const void* data = reinterpret_cast<const void*>(payload + 1);

                switch (payload->ptype) {
                    case SceneCmd_PropType::PlainData: {
                    } break;
                    case SceneCmd_PropType::Entity: {
                        DEV_ASSERT(payload->data_size >= payload->ele_count * sizeof(Entity));
                        // @HACK: cast away const to resolve entity
                        Entity* e = const_cast<Entity*>((Entity*)data);
                        for (uint32_t i = 0; i < payload->ele_count; ++i) {
                            e[i] = Resolve(e[i]);
                        }
                    } break;
                    default: {
                        CRASH_NOW();
                    } break;
                }

                p_mut.ChangeProperty(Resolve(payload->ent),
                                     payload->cid,
                                     payload->pid,
                                     data,
                                     payload->data_size);
            } break;
            default: {
                CRASH_NOW_MSG("Invalid opcode");
            } break;
        }
        p += header->size;
    }

    m_bytes.clear();
    m_next_entity = kTmpBase;
}

void SceneCommandBuffer::SetRemap(Entity p_temp, Entity p_real) {
    const uint32_t index = p_temp.GetId() - kTmpBase;
    DEV_ASSERT(index < m_remap.size());
    m_remap[index] = p_real;
}

void SceneCommandBuffer::WriteEntityRecord(SceneCmd_Op p_op, Entity p_ent) {
    SceneCmd_Header header{
        .op = p_op,
        .flags = 0,
        .size = uint16_t(sizeof(SceneCmd_Header) + sizeof(p_ent)),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    std::memcpy(m_bytes.data() + old, &header, sizeof(header));
    std::memcpy(m_bytes.data() + old + sizeof(header), &p_ent, sizeof(p_ent));
}

void SceneCommandBuffer::WriteComponentRecord(SceneCmd_Op p_op,
                                              Entity p_ent,
                                              BuiltinComponentId p_cid) {
    SceneCmd_Header header{
        .op = p_op,
        .flags = 0,
        .size = uint16_t(sizeof(SceneCmd_Header) + sizeof(SceneCmd_PayloadComponent)),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    SceneCmd_PayloadComponent payload{ p_ent, p_cid };
    std::memcpy(out, &payload, sizeof(payload));
}

void SceneCommandBuffer::WritePropertyRecord(SceneCmd_Op p_op,
                                             Entity p_ent,
                                             BuiltinComponentId p_cid,
                                             PropertyId p_pid,
                                             const void* p_data,
                                             uint32_t p_data_size,
                                             SceneCmd_PropType p_ptype,
                                             uint32_t p_ele_count) {

    SceneCmd_Header header{
        .op = p_op,
        .flags = 0,
        .size = uint16_t(sizeof(SceneCmd_Header) + sizeof(SceneCmd_PayloadProperty) + p_data_size),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    SceneCmd_PayloadProperty payload{
        .ent = p_ent,
        .cid = p_cid,
        .ptype = p_ptype,
        .pid = p_pid,
        .data_size = p_data_size,
        .ele_count = p_ele_count,
    };
    std::memcpy(out, &payload, sizeof(payload));

    out += sizeof(payload);
    std::memcpy(out, p_data, p_data_size);
}

}  // namespace cave