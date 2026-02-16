#include "EditObjectCmd.h"

#include "cave/runtime/scene/SceneCommandWriter.h"
#include "cave/runtime/scene/SceneCommandExecutor.h"

#include "engine/private/runtime/scene/Scene.h"
#include "engine/private/runtime/framework/AssetRegistry.h"

#include "editor/Enums.h"

namespace cave {

[[maybe_unused]] static std::string GenerateName(std::string_view p_name) {
    static int s_counter = 0;
    return std::format("{}-{}", p_name, ++s_counter);
}

bool AddObjectCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            SceneCommandWriter cb(AssetRegistry::GetSingleton());
            ecs::Entity created{};
            switch (m_type) {
#define ENTITY_TYPE(NAME, ...)                                  \
    case EntityType::NAME: {                                    \
        created = cb.Create##NAME##Object(GenerateName(#NAME)); \
    } break;
                ENTITY_TYPE_LIST
#undef ENTITY_TYPE
                default:
                    LOG_FATAL("Entity type {} not supported", static_cast<int>(m_type));
                    break;
            }

            SceneCommandExecutor executor(*scene);
            executor.Playback(cb);
            m_created = executor.Resolve(created);

            ecs::Entity parent = m_ent;
            if (scene->m_root.IsValid()) {
                scene->AttachChild(m_created, parent.IsValid() ? parent : scene->m_root);
            } else {
                scene->m_root = m_created;
            }

            return true;
        }
    }

    return false;
}

bool AddObjectCmd::Undo(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            scene->RemoveEntity(m_created);
            m_created = ecs::Entity::Null();
            return true;
        }
    }
    return false;
}

bool DeleteObjectCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            scene->RemoveEntity(m_ent);
            return true;
        }
    }
    return false;
}

bool DeleteObjectCmd::Undo(IDocument&) {
    LOG_WARN("TODO: implement DeleteObjectCmd::Undo");
    return false;
}

bool CloneObjectCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            scene->DuplicateEntity(m_ent);
            return true;
        }
    }
    return true;
}

bool CloneObjectCmd::Undo(IDocument&) {
    LOG_WARN("TODO: implement CloneObjectCmd::Undo");
    return false;
}

}  // namespace cave
