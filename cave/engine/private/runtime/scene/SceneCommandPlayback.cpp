#include "cave/runtime/scene/SceneCommandPlayback.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;

EntityMap::EntityMap(uint32_t reserve) {
    m_remap.clear();
    m_remap.resize(reserve, Entity::null());
}

static bool IsTemp(ecs::Entity ent) noexcept { return ent.id() >= kSceneCmdTmpBase; }

Entity EntityMap::resolve(Entity ent) const noexcept {
    if (!IsTemp(ent)) return ent;

    const uint32_t index = ent.id() - kSceneCmdTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    CRASH_NOW();
    return Entity::null();
}

void EntityMap::setRemap(Entity tmp, Entity real) {
    const uint32_t index = tmp.id() - kSceneCmdTmpBase;
    DEV_ASSERT(index < m_remap.size());
    m_remap[index] = real;
}

void SceneCommandPlayback::Play(SceneCommandBuffer& cmd_buffer,
                                ISceneCommandExecutor& executor,
                                const Context& ctx) {
    const uint8_t* p = cmd_buffer.bytes();
    if (p == nullptr) {
        return;
    }

    const uint8_t* end = p + cmd_buffer.byteSize();

    auto& map = ctx.map;

    while (p < end) {
        const SceneCmd_Header* header = reinterpret_cast<const SceneCmd_Header*>(p);
        const uint8_t* payload_raw = reinterpret_cast<const uint8_t*>(header + 1);

        switch (header->op) {
            case SceneCmd_Op::CreateEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                Entity real = ctx.scene.createEntity();
                map.setRemap(e, real);
            } break;
            case SceneCmd_Op::DestroyEntity: {
                // const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                CRASH_NOW_MSG("not supported");
            } break;
            case SceneCmd_Op::AddComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                executor.addComponent(map.resolve(payload->ent), payload->cid);
            } break;
            case SceneCmd_Op::RemoveComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                executor.removeComponent(map.resolve(payload->ent), payload->cid);
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
                            e[i] = map.resolve(e[i]);
                        }
                    } break;
                    default: {
                        CRASH_NOW();
                    } break;
                }

                // @TODO: generate Undoable Command ChangeProperty
                executor.changeProperty(map.resolve(payload->ent),
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
}

}  // namespace cave
