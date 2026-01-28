#include "EditService.h"

#include "editor/document/DocumentService.h"
#include "editor/services/Workspace.h"

#include "engine/private/debugger/profiler.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/EntityFactory.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"

#include "editor/EditorState.h"
#include "editor/viewer/Viewer.h"

#include "../Enums.h"

namespace cave {

// @TODO: deprecate commands
EditService::ICommand::ICommand(EditorState& p_editor,
                                SceneId p_scene_id)
    : m_editor(p_editor)
    , m_scene_id(p_scene_id) {
}

Scene* EditService::ICommand::ResolveScene() {
    return m_editor.GetApp().GetSceneRegistry()->Resolve(m_scene_id);
}

void EditService::Submit(DocId p_doc_id, std::unique_ptr<IEditCmd> p_cmd) {
    m_pending_cmds[p_doc_id].emplace_back(std::move(p_cmd));
}

void EditService::Undo(DocId p_doc_id) {
    if (IDocument* doc = ResolveDoc(p_doc_id)) {
        doc->Undo();
    }
}

void EditService::Redo(DocId p_doc_id) {
    if (IDocument* doc = ResolveDoc(p_doc_id)) {
        doc->Redo();
    }
}

bool EditService::CanUndo(DocId p_doc_id) {
    if (IDocument* doc = ResolveDoc(p_doc_id)) {
        return doc->CanUndo();
    }

    return false;
}

bool EditService::CanRedo(DocId p_doc_id) {
    if (IDocument* doc = ResolveDoc(p_doc_id)) {
        return doc->CanRedo();
    }

    return false;
}

void EditService::FlushPendingCmds() {
    CAVE_PROFILE_EVENT();

    for (auto&& [doc_id, pending] : m_pending_cmds) {
        IDocument* doc = ResolveDoc(doc_id);
        if (DEV_VERIFY(doc)) {
            for (int i = (int)pending.size() - 1; i >= 0; --i) {
                doc->Apply(std::move(pending[i]), 0);
            }
        }
    }

    m_pending_cmds.clear();
}

IDocument* EditService::ResolveDoc(DocId p_doc_id) {
    return m_editor.DocumentService().Resolve(p_doc_id);
}

// @TODO: refactor
static std::string GenerateName(std::string_view p_name) {
    static int s_counter = 0;
    return std::format("{}-{}", p_name, ++s_counter);
}

class CreateObjectCommand : public EditService::ICommand {
public:
    CreateObjectCommand(EditorState& p_editor,
                        SceneId p_scene_id,
                        EntityType p_type,
                        ecs::Entity p_parent)
        : EditService::ICommand(p_editor, p_scene_id)
        , m_type(p_type)
        , m_parent(p_parent) {}

    bool Undo() override {
        return false;
    }

    bool Redo() override {
        Scene* scene = ResolveScene();
        if (!scene) return false;

        ecs::Entity id;
        switch (m_type) {
#define ENTITY_TYPE(NAME, ...)                                                 \
    case EntityType::NAME: {                                                   \
        id = EntityFactory::Create##NAME##Entity(*scene, GenerateName(#NAME)); \
    } break;
            ENTITY_TYPE_LIST
#undef ENTITY_TYPE
            default:
                LOG_FATAL("Entity type {} not supported", static_cast<int>(m_type));
                break;
        }

        if (scene->m_root.IsValid()) {
            scene->AttachChild(id, m_parent.IsValid() ? m_parent : scene->m_root);
        } else {
            scene->m_root = id;
        }

        return true;
    }

protected:
    EntityType m_type;
    ecs::Entity m_parent;
};

class DeleteObjectCommand : public EditService::ICommand {
public:
    DeleteObjectCommand(EditorState& p_editor,
                        SceneId p_scene_id,
                        ecs::Entity p_target)
        : EditService::ICommand(p_editor, p_scene_id)
        , m_target(p_target) {}

    bool Undo() override {
        return false;
    }

    bool Redo() override {
        Scene* scene = ResolveScene();
        DEV_ASSERT(scene);
        DEV_ASSERT(m_target.IsValid());
        scene->RemoveEntity(m_target);
        return true;
    }

private:
    ecs::Entity m_target;
};

class CloneObjectCommand : public EditService::ICommand {
public:
    CloneObjectCommand(EditorState& p_editor,
                       SceneId p_scene_id,
                       ecs::Entity p_target)
        : EditService::ICommand(p_editor, p_scene_id)
        , m_target(p_target) {}

    bool Undo() override {
        return false;
    }

    bool Redo() override {
        Scene* scene = ResolveScene();
        DEV_ASSERT(scene);
        scene->DuplicateEntity(m_target);
        return true;
    }

private:
    ecs::Entity m_target;
};

EditService::EditService(EditorState& p_editor)
    : m_editor(p_editor) {}

void EditService::CommandCreateObject(SceneId p_scene_id,
                                      EntityType p_type,
                                      ecs::Entity p_parent) {
    m_old_pending_commands.emplace_back(std::make_unique<CreateObjectCommand>(
        m_editor,
        p_scene_id,
        p_type,
        p_parent));
}

void EditService::CommandDeleteObject(SceneId p_scene_id,
                                      ecs::Entity p_target) {
    m_old_pending_commands.emplace_back(std::make_unique<DeleteObjectCommand>(
        m_editor,
        p_scene_id,
        p_target));
}

void EditService::CommandCloneObject(SceneId p_scene_id,
                                     ecs::Entity p_target) {
    m_old_pending_commands.emplace_back(std::make_unique<CloneObjectCommand>(
        m_editor,
        p_scene_id,
        p_target));
}

}  // namespace cave
