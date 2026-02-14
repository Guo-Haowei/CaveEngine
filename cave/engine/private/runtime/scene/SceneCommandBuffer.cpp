#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneEdit.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

ecs::Entity SceneCommandBuffer::Resolve(ecs::Entity p_entity) const noexcept {
    if (!IsTemp(p_entity)) return p_entity;

    const uint32_t index = p_entity.GetId() - kTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    return ecs::Entity::Null();
}

void SceneCommandBuffer::Playback(Scene& p_scene) {
    m_remap.clear();
    m_remap.resize(m_next_entity - kTmpBase, ecs::Entity::Null());

    const uint8_t* p = m_bytes.data();
    const uint8_t* end = m_bytes.data() + m_bytes.size();

    SceneEdit edit(p_scene);

    while (p < end) {
        const Header* header = reinterpret_cast<const Header*>(p);
        const uint8_t* payload = reinterpret_cast<const uint8_t*>(header + 1);

        switch (header->op) {
            case Op::CreateEntity: {
                const auto* create = reinterpret_cast<const Payload_Create*>(payload);
                ecs::Entity real = edit.CreateEntity();
                SetRemap(create->out_temp, real);
            } break;
            case Op::DestroyEntity: {
                const auto* destroy = reinterpret_cast<const Payload_Destroy*>(payload);
                ecs::Entity real = Resolve(destroy->entity);
                if (DEV_VERIFY(real.IsValid())) {
                    edit.DestroyEntity(real);
                }
            } break;
            case Op::AddComponent:
            case Op::RemoveComponent: {
                const auto* pc = reinterpret_cast<const Payload_Component*>(payload);
                ecs::Entity real = Resolve(pc->entity);
                DispatchComponentOp(p_scene,
                                    header->op,
                                    real,
                                    pc->type);
            } break;
            case Op::ChangeProperty: {
                const auto* pp = reinterpret_cast<const Payload_Property*>(payload);
                ecs::Entity real = Resolve(pp->entity);
                const void* data = reinterpret_cast<const void*>(pp + 1);
                DispatchPropertyOp(p_scene,
                                   real,
                                   *pp,
                                   data);
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

void SceneCommandBuffer::SetRemap(ecs::Entity p_temp, ecs::Entity p_real) {
    const uint32_t index = p_temp.GetId() - kTmpBase;
    DEV_ASSERT(index < m_remap.size());
    m_remap[index] = p_real;
}

void SceneCommandBuffer::WriteEntityRecord(Op p_op,
                                           const void* p_payload,
                                           uint16_t p_payload_size) {
    Header header{
        .op = p_op,
        .size = uint16_t(sizeof(Header) + p_payload_size),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    std::memcpy(m_bytes.data() + old, &header, sizeof(header));
    std::memcpy(m_bytes.data() + old + sizeof(header), p_payload, p_payload_size);
}

void SceneCommandBuffer::WriteComponentRecord(Op p_op,
                                              ecs::Entity p_entity,
                                              BuildInComponentId p_type) {
    Header header{
        .op = p_op,
        .size = uint16_t(sizeof(Header) + sizeof(Payload_Component)),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    Payload_Component payload{ p_entity, p_type };
    std::memcpy(out, &payload, sizeof(payload));
}

void SceneCommandBuffer::WritePropertyRecord(Op p_op,
                                             ecs::Entity p_entity,
                                             BuildInComponentId p_type,
                                             std::string_view p_property,
                                             const void* p_data,
                                             uint32_t p_data_size) {
    DEV_ASSERT_MSG(p_property.size() < kPropertyNameMax, "Property name overflow");

    Header header{
        .op = p_op,
        .size = uint16_t(sizeof(Header) + sizeof(Payload_Property) + p_data_size),
    };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + header.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &header, sizeof(header));
    out += sizeof(header);

    Payload_Property payload{
        .entity = p_entity,
        .type = p_type,
        .property_name = p_property,
        .data_size = p_data_size,
    };
    std::memcpy(out, &payload, sizeof(payload));

    out += sizeof(payload);
    std::memcpy(out, p_data, p_data_size);
}

void SceneCommandBuffer::DispatchComponentOp(Scene& p_scene,
                                             Op p_op,
                                             ecs::Entity p_entity,
                                             BuildInComponentId p_type_id) {
    if (p_op == Op::AddComponent) {
        switch (p_type_id) {
#define REGISTER_COMPONENT(T, ...)   \
    case T##_Id: {                   \
        p_scene.Create<T>(p_entity); \
    } break;
            REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT
            default: {
                CRASH_NOW_MSG("Unknown component type");
            } break;
        }
        return;
    }

    if (p_op == Op::RemoveComponent) {
        switch (p_type_id) {
#define REGISTER_COMPONENT(T, ...)   \
    case T##_Id: {                   \
        p_scene.Remove<T>(p_entity); \
    } break;
            REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT
            default: {
                CRASH_NOW_MSG("Unknown component type");
            } break;
        }
        return;
    }

    CRASH_NOW();
}

void SceneCommandBuffer::DispatchPropertyOp(Scene& p_scene,
                                            ecs::Entity p_entity,
                                            const Payload_Property& p_payload,
                                            const void* p_data) {
    SceneEdit edit(p_scene);
    switch (p_payload.type) {
#define REGISTER_COMPONENT(T, ...)                                                                  \
    case T##_Id: {                                                                                  \
        edit.ModifyField<T>(p_entity, p_payload.property_name.view(), p_data, p_payload.data_size); \
    } break;
        REGISTER_COMPONENT_SERIALIZED_LIST
#undef REGISTER_COMPONENT
        default: {
            CRASH_NOW_MSG("Unknown component type");
        } break;
    }
}

}  // namespace cave
