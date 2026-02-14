#include "EditObjectCmd.h"

#include "cave/runtime/scene/SceneEdit.h"

#include "engine/private/runtime/scene/EntityFactory.h"

#include "editor/Enums.h"

namespace cave {

// @TODO: refactor
static std::string GenerateName(std::string_view p_name) {
    static int s_counter = 0;
    return std::format("{}-{}", p_name, ++s_counter);
}

bool AddObjectCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            SceneEdit edit(*scene);
            switch (m_type) {
#define ENTITY_TYPE(NAME, ...)                                                        \
    case EntityType::NAME: {                                                          \
        m_created = EntityFactory::Create##NAME##Entity(*scene, GenerateName(#NAME)); \
    } break;
                ENTITY_TYPE_LIST
#undef ENTITY_TYPE
                default:
                    LOG_FATAL("Entity type {} not supported", static_cast<int>(m_type));
                    break;
            }

            ecs::Entity parent = m_entity;
            if (scene->m_root.IsValid()) {
                edit.AttachChild(m_created, parent.IsValid() ? parent : scene->m_root);
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
            SceneEdit edit(*scene);
            edit.DestroyEntity(m_created);
            m_created = ecs::Entity::Null();
            return true;
        }
    }
    return false;
}

bool DeleteObjectCmd::Do(IDocument& p_doc) {
    if (SceneDocument* scene_doc = dynamic_cast<SceneDocument*>(&p_doc)) {
        if (Scene* scene = ResolveScene(scene_doc->GetPreviewScene())) {
            SceneEdit edit(*scene);
            edit.DestroyEntity(m_entity);
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
            scene->DuplicateEntity(m_entity);
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
