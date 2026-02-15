#include "cave/runtime/scene/SceneCommandBuffer.h"
#include "cave/runtime/scene/SceneMutator.h"

#include "engine/private/runtime/ecs/components/All.h"
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

ecs::Entity SceneCommandBuffer::Resolve(ecs::Entity p_ent) const noexcept {
    if (!IsTemp(p_ent)) return p_ent;

    const uint32_t index = p_ent.GetId() - kTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    return ecs::Entity::Null();
}

void SceneCommandBuffer::Playback(SceneMutator& p_mut) {
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
                ecs::Entity real = p_mut.CreateEntity();
                SetRemap(create->out_temp, real);
            } break;
            case Op::DestroyEntity: {
                const auto* destroy = reinterpret_cast<const Payload_Destroy*>(payload);
                p_mut.RemoveEntity(Resolve(destroy->ent));
            } break;
            case Op::AddComponent: {
                const auto* pc = reinterpret_cast<const Payload_Component*>(payload);
                p_mut.AddComponent(Resolve(pc->ent), pc->type);
            } break;
            case Op::RemoveComponent: {
                const auto* pc = reinterpret_cast<const Payload_Component*>(payload);
                p_mut.RemoveComponent(Resolve(pc->ent), pc->type);
            } break;
            case Op::ChangeProperty: {
                const auto* pp = reinterpret_cast<const Payload_Property*>(payload);
                const void* data = reinterpret_cast<const void*>(pp + 1);
                p_mut.ChangeProperty(Resolve(pp->ent),
                                     pp->type,
                                     pp->prop_id,
                                     data,
                                     pp->data_size);
            } break;
            case Op::AttachRoot: {
                const auto* attach = reinterpret_cast<const Payload_AttachRoot*>(payload);
                p_mut.GetScene().AttachChild(Resolve(attach->ent));
            } break;
            default: {
                CRASH_NOW_MSG("Invalid opcode");
            } break;
        }
        p += header->size;
    }

    Scene& scene = p_mut.GetScene();
    // @NOTE: resolve mesh and hierachy
    for (auto [ent, hier] : scene.View<HierarchyComponent>()) {
        hier.parent_id = Resolve(hier.parent_id);
    }
    for (auto [ent, mesh] : scene.View<MeshRendererComponent>()) {
        auto& mats = mesh.GetMaterialInstances();
        for (ecs::Entity& mat : mats) {
            mat = Resolve(mat);
        }
        mesh.SetSkeletonId(Resolve(mesh.GetSkeletonId()));
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
                                              ecs::Entity p_ent,
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

    Payload_Component payload{ p_ent, p_type };
    std::memcpy(out, &payload, sizeof(payload));
}

void SceneCommandBuffer::WritePropertyRecord(Op p_op,
                                             ecs::Entity p_ent,
                                             BuildInComponentId p_type,
                                             PropertyId p_prop_id,
                                             const void* p_data,
                                             uint32_t p_data_size) {
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
        .ent = p_ent,
        .type = p_type,
        .prop_id = p_prop_id,
        .data_size = p_data_size,
    };
    std::memcpy(out, &payload, sizeof(payload));

    out += sizeof(payload);
    std::memcpy(out, p_data, p_data_size);
}

}  // namespace cave