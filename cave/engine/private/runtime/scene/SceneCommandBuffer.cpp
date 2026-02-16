#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneMutator.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;

struct SceneCommandBuffer::Header {
    SceneCommandOp op;
    uint8_t flags;
    uint16_t size;
};

struct SceneCommandBuffer::Payload_Entity {
    Entity ent;
};

struct SceneCommandBuffer::Payload_Component {
    Entity ent;
    BuiltinComponentId cid;
};

struct SceneCommandBuffer::Payload_Property {
    Entity ent;
    BuiltinComponentId cid;
    PropType ptype;
    PropertyId pid;
    uint32_t data_size;
};

Entity SceneCommandBuffer::CreateEntity() {
    Entity e = AllocateTempEntity();
    WriteEntityRecord(SceneCommandOp::CreateEntity, e);
    return e;
}

void SceneCommandBuffer::DestroyEntity(Entity p_ent) {
    WriteEntityRecord(SceneCommandOp::DestroyEntity, p_ent);
}

void SceneCommandBuffer::AddComponent(Entity p_ent, BuiltinComponentId p_id) {
    WriteComponentRecord(SceneCommandOp::AddComponent, p_ent, p_id);
}

void SceneCommandBuffer::RemoveComponent(Entity p_ent, BuiltinComponentId p_id) {
    WriteComponentRecord(SceneCommandOp::RemoveComponent, p_ent, p_id);
}

void SceneCommandBuffer::SetProperty(Entity p_ent,
                                     BuiltinComponentId p_cid,
                                     const PropertyId& p_pid,
                                     const Entity& p_value) {
    WritePropertyRecord(SceneCommandOp::ChangeProperty,
                        p_ent,
                        p_cid,
                        p_pid,
                        &p_value,
                        sizeof(Entity),
                        PropType::Entity);
}

Entity SceneCommandBuffer::Resolve(Entity p_ent) const noexcept {
    if (!IsTemp(p_ent)) return p_ent;

    const uint32_t index = p_ent.GetId() - kTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    return Entity::Null();
}

void SceneCommandBuffer::Playback(SceneMutator& p_mut) {
    m_remap.clear();
    m_remap.resize(m_next_entity - kTmpBase, Entity::Null());

    const uint8_t* p = m_bytes.data();
    const uint8_t* end = m_bytes.data() + m_bytes.size();

    while (p < end) {
        const Header* header = reinterpret_cast<const Header*>(p);
        const uint8_t* payload_raw = reinterpret_cast<const uint8_t*>(header + 1);

        switch (header->op) {
            case SceneCommandOp::CreateEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                Entity real = p_mut.CreateEntity();
                SetRemap(e, real);
            } break;
            case SceneCommandOp::DestroyEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                p_mut.RemoveEntity(Resolve(e));
            } break;
            case SceneCommandOp::AddComponent: {
                const auto* payload = reinterpret_cast<const Payload_Component*>(payload_raw);
                p_mut.AddComponent(Resolve(payload->ent), payload->cid);
            } break;
            case SceneCommandOp::RemoveComponent: {
                const auto* payload = reinterpret_cast<const Payload_Component*>(payload_raw);
                p_mut.RemoveComponent(Resolve(payload->ent), payload->cid);
            } break;
            case SceneCommandOp::ChangeProperty: {
                const auto* payload = reinterpret_cast<const Payload_Property*>(payload_raw);
                const void* data = reinterpret_cast<const void*>(payload + 1);

                Entity resolved;
                switch (payload->ptype) {
                    case PropType::PlainData: {
                    } break;
                    case PropType::Entity: {
                        DEV_ASSERT(payload->data_size == sizeof(Entity));
                        resolved = Resolve(*reinterpret_cast<const Entity*>(data));
                        data = &resolved;
                    } break;
                    case PropType::EntityArray: {
                        // @TODO: resolve
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

    Scene& scene = p_mut.GetScene();

    // @NOTE: resolve mesh and hierachy
    for (auto [ent, mesh] : scene.View<MeshRendererComponent>()) {
        auto& mats = mesh.GetMaterialInstances();
        for (Entity& mat : mats) {
            mat = Resolve(mat);
        }
        mesh.SetSkeletonId(Resolve(mesh.GetSkeletonId()));
    }

    m_bytes.clear();
    m_next_entity = kTmpBase;
}

void SceneCommandBuffer::SetRemap(Entity p_temp, Entity p_real) {
    const uint32_t index = p_temp.GetId() - kTmpBase;
    DEV_ASSERT(index < m_remap.size());
    m_remap[index] = p_real;
}

void SceneCommandBuffer::WriteEntityRecord(SceneCommandOp p_op, Entity p_ent) {
    Header header{
        .op = p_op,
        .flags = 0,
        .size = uint16_t(sizeof(Header) + sizeof(p_ent)),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    std::memcpy(m_bytes.data() + old, &header, sizeof(header));
    std::memcpy(m_bytes.data() + old + sizeof(header), &p_ent, sizeof(p_ent));
}

void SceneCommandBuffer::WriteComponentRecord(SceneCommandOp p_op,
                                              Entity p_ent,
                                              BuiltinComponentId p_cid) {
    Header header{
        .op = p_op,
        .flags = 0,
        .size = uint16_t(sizeof(Header) + sizeof(Payload_Component)),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    Payload_Component payload{ p_ent, p_cid };
    std::memcpy(out, &payload, sizeof(payload));
}

void SceneCommandBuffer::WritePropertyRecord(SceneCommandOp p_op,
                                             Entity p_ent,
                                             BuiltinComponentId p_cid,
                                             PropertyId p_pid,
                                             const void* p_data,
                                             uint32_t p_data_size,
                                             PropType p_ptype) {
    Header header{
        .op = p_op,
        .flags = 0,
        .size = uint16_t(sizeof(Header) + sizeof(Payload_Property) + p_data_size),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    Payload_Property payload{
        .ent = p_ent,
        .cid = p_cid,
        .ptype = p_ptype,
        .pid = p_pid,
        .data_size = p_data_size,
    };
    std::memcpy(out, &payload, sizeof(payload));

    out += sizeof(payload);
    std::memcpy(out, p_data, p_data_size);
}

}  // namespace cave