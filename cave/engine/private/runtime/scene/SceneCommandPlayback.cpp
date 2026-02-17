#include "cave/runtime/scene/SceneCommandPlayback.h"

#include "engine/private/runtime/scene/Scene.h"

namespace cave {

using ecs::Entity;

EntityMap::EntityMap(uint32_t p_reserve) {
    m_remap.clear();
    m_remap.resize(p_reserve, Entity::Null());
}

static bool IsTemp(ecs::Entity p_id) noexcept { return p_id.GetId() >= kSceneCmdTmpBase; }

Entity EntityMap::Resolve(Entity p_ent) const noexcept {
    if (!IsTemp(p_ent)) return p_ent;

    const uint32_t index = p_ent.GetId() - kSceneCmdTmpBase;
    if (DEV_VERIFY(index < m_remap.size())) {
        return m_remap[index];
    }

    return Entity::Null();
}

void EntityMap::SetRemap(Entity p_temp, Entity p_real) {
    const uint32_t index = p_temp.GetId() - kSceneCmdTmpBase;
    DEV_ASSERT(index < m_remap.size());
    m_remap[index] = p_real;
}

void SceneCommandPlayback::Play(SceneCommandBuffer& p_cb,
                                ISceneCommandExecutor& p_exe,
                                const Context& p_ctx) {
    const uint8_t* p = p_cb.Data();
    if (p == nullptr) {
        return;
    }

    const uint8_t* end = p + p_cb.Size();

    auto& map = p_ctx.map;

    while (p < end) {
        const SceneCmd_Header* header = reinterpret_cast<const SceneCmd_Header*>(p);
        const uint8_t* payload_raw = reinterpret_cast<const uint8_t*>(header + 1);

        switch (header->op) {
            case SceneCmd_Op::CreateEntity: {
                const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                Entity real = p_ctx.scene.CreateEntity();
                map.SetRemap(e, real);
            } break;
            case SceneCmd_Op::DestroyEntity: {
                // const Entity& e = *reinterpret_cast<const Entity*>(payload_raw);
                CRASH_NOW_MSG("not supported");
            } break;
            case SceneCmd_Op::AddComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                p_exe.AddComponent(map.Resolve(payload->ent), payload->cid);
            } break;
            case SceneCmd_Op::RemoveComponent: {
                const auto* payload = reinterpret_cast<const SceneCmd_PayloadComponent*>(payload_raw);
                p_exe.RemoveComponent(map.Resolve(payload->ent), payload->cid);
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
                            e[i] = map.Resolve(e[i]);
                        }
                    } break;
                    default: {
                        CRASH_NOW();
                    } break;
                }

                // @TODO: generate Undoable Command ChangeProperty
                p_exe.ChangeProperty(map.Resolve(payload->ent),
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
