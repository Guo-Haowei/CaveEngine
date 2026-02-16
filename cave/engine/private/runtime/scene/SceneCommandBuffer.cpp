#include "cave/runtime/scene/SceneCommandBuffer.h"

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