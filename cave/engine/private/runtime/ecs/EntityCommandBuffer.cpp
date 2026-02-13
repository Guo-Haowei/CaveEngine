#include "cave/runtime/ecs/EntityCommandBuffer.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

ecs::Entity EntityCommandBuffer::Resolve(ecs::Entity p_entity) const noexcept {
    if (!IsTemp(p_entity)) return p_entity;

    const uint32_t index = p_entity.GetId() - kTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    return ecs::Entity::Null();
}

void EntityCommandBuffer::Playback(Scene& p_scene) {
    m_remap.clear();
    m_remap.resize(m_next_entity - kTmpBase, ecs::Entity::Null());

    const uint8_t* p = m_bytes.data();
    const uint8_t* end = m_bytes.data() + m_bytes.size();

    while (p < end) {
        const Header* header = reinterpret_cast<const Header*>(p);
        const uint8_t* payload = reinterpret_cast<const uint8_t*>(header + 1);

        switch (header->op) {
            case Op::CreateEntity: {
                const auto* create = reinterpret_cast<const Payload_Create*>(payload);
                ecs::Entity real = p_scene.CreateEntity();
                SetRemap(create->out_temp, real);
            } break;
            case Op::DestroyEntity: {
                const auto* destroy = reinterpret_cast<const Payload_Destroy*>(payload);
                ecs::Entity real = Resolve(destroy->entity);
                if (DEV_VERIFY(real.IsValid())) {
                    p_scene.RemoveEntity(real);
                }
            } break;
            case Op::AddComponent:
            case Op::SetComponent:
            case Op::RemoveComponent: {
                const auto* c = reinterpret_cast<const Payload_Component*>(payload);
                ecs::Entity real = Resolve(c->entity);
                DispatchComponentOp(p_scene,
                                    header->op,
                                    real,
                                    c->type,
                                    payload + sizeof(Payload_Component),
                                    c->data_size);
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

void EntityCommandBuffer::SetRemap(ecs::Entity p_temp, ecs::Entity p_real) {
    const uint32_t index = p_temp.GetId() - kTmpBase;
    DEV_ASSERT(index < m_remap.size());
    m_remap[index] = p_real;
}

void EntityCommandBuffer::WriteRecord(Op p_op,
                                      const void* p_payload,
                                      uint16_t p_payload_size) {
    Header h{ p_op, 0, uint16_t(sizeof(Header) + p_payload_size) };
    const size_t old = m_bytes.size();
    m_bytes.resize(old + h.size);
    std::memcpy(m_bytes.data() + old, &h, sizeof(h));
    std::memcpy(m_bytes.data() + old + sizeof(h), p_payload, p_payload_size);
}

void EntityCommandBuffer::WriteComponentRecord(Op p_op,
                                               ecs::Entity p_entity,
                                               ComponentId p_type,
                                               const void* p_data,
                                               uint32_t p_data_size) {
    Header h{ p_op, 0, uint16_t(sizeof(Header) + sizeof(Payload_Component) + p_data_size) };

    const size_t old = m_bytes.size();
    m_bytes.resize(old + h.size);

    uint8_t* out = m_bytes.data() + old;
    std::memcpy(out, &h, sizeof(h));
    out += sizeof(h);

    Payload_Component c{ p_entity, p_type, p_data_size };
    std::memcpy(out, &c, sizeof(c));
    out += sizeof(c);

    if (p_data_size) {
        std::memcpy(out, p_data, p_data_size);
    }
}

void EntityCommandBuffer::DispatchComponentOp(Scene& p_scene,
                                              Op p_op,
                                              ecs::Entity p_entity,
                                              ComponentId p_type_id,
                                              const void* p_data,
                                              uint32_t p_size) {
    if (p_op == Op::AddComponent) {
        switch (p_type_id) {
#define REGISTER_COMPONENT(T, ...)                       \
    case T##_Id: {                                       \
        DEV_ASSERT(sizeof(T) == p_size);                 \
        T& component = p_scene.Create<T>(p_entity);      \
        component = *reinterpret_cast<const T*>(p_data); \
    } break;
            REGISTER_COMPONENT_LIST
#undef REGISTER_COMPONENT
            default: {
                CRASH_NOW_MSG("Unknown component type");
            } break;
        }
        return;
    }

    CRASH_NOW();
}

}  // namespace cave
