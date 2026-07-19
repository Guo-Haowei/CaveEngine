#include "cave/runtime/scene/SceneCommandBuffer.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;

Entity SceneCommandBuffer::createEntity() {
    Entity e = allocateTempEntity();
    writeEntityRecord(SceneCmd_Op::CreateEntity, e);
    return e;
}

void SceneCommandBuffer::destroyEntity(Entity ent) {
    writeEntityRecord(SceneCmd_Op::DestroyEntity, ent);
}

void SceneCommandBuffer::addComponent(Entity ent, ComponentId cid) {
    writeComponentRecord(SceneCmd_Op::AddComponent, ent, cid);
}

void SceneCommandBuffer::removeComponent(Entity ent, ComponentId cid) {
    writeComponentRecord(SceneCmd_Op::RemoveComponent, ent, cid);
}

void SceneCommandBuffer::setProperty(Entity ent,
                                     ComponentId cid,
                                     const PropertyId& pid,
                                     const Entity& value) {
    writePropertyRecord(SceneCmd_Op::AssignProperty,
                        ent,
                        cid,
                        pid,
                        &value,
                        sizeof(Entity),
                        SceneCmd_PropType::Entity);
}

void SceneCommandBuffer::writeEntityRecord(SceneCmd_Op op, Entity ent) {
    SceneCmd_Header header{
        .op = op,
        .flags = 0,
        .size = uint16_t(sizeof(SceneCmd_Header) + sizeof(ent)),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    std::memcpy(m_bytes.data() + old, &header, sizeof(header));
    std::memcpy(m_bytes.data() + old + sizeof(header), &ent, sizeof(ent));
}

void SceneCommandBuffer::writeComponentRecord(SceneCmd_Op op,
                                              Entity ent,
                                              ComponentId cid) {
    SceneCmd_Header header{
        .op = op,
        .flags = 0,
        .size = uint16_t(sizeof(SceneCmd_Header) + sizeof(SceneCmd_PayloadComponent)),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    SceneCmd_PayloadComponent payload{ ent, cid };
    std::memcpy(out, &payload, sizeof(payload));
}

void SceneCommandBuffer::writePropertyRecord(SceneCmd_Op op,
                                             Entity ent,
                                             ComponentId cid,
                                             PropertyId pid,
                                             const void* data,
                                             uint32_t data_size,
                                             SceneCmd_PropType prop_type,
                                             uint32_t element_count) {

    SceneCmd_Header header{
        .op = op,
        .flags = 0,
        .size = uint16_t(sizeof(SceneCmd_Header) + sizeof(SceneCmd_PayloadProperty) + data_size),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    SceneCmd_PayloadProperty payload{
        .ent = ent,
        .cid = cid,
        .ptype = prop_type,
        .pid = pid,
        .data_size = data_size,
        .ele_count = element_count,
    };
    std::memcpy(out, &payload, sizeof(payload));

    out += sizeof(payload);
    std::memcpy(out, data, data_size);
}

}  // namespace cave