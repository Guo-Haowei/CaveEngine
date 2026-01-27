#include "EditService.h"

#include "engine/private/debugger/profiler.h"
#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/EntityFactory.h"
#include "engine/private/runtime/scene/SceneManager.h"

#include "editor/EditorState.h"
#include "editor/viewer/Viewer.h"

#include "../Enums.h"

namespace cave {

// @TODO: move the commands somewhere else
EditService::ICommand::ICommand(EditorState& p_editor,
                                SceneId p_scene_id)
    : m_editor(p_editor)
    , m_scene_id(p_scene_id) {
}

Scene* EditService::ICommand::ResolveScene() {
    return m_editor.GetApp().GetSceneManager()->Resolve(m_scene_id);
}

// @TODO: move this to document service
class OpenDocumentCommand : public EditService::ICommand {
public:
    OpenDocumentCommand(EditorState& p_editor,
                        SceneId p_scene_id,
                        const Guid& p_guid)
        : EditService::ICommand(p_editor, p_scene_id)
        , m_guid(p_guid) {}

    bool Undo() override {
        return false;
    }

    bool Redo() override {
        auto asset_registry = m_editor.GetApp().GetAssetRegistry();
        if (auto res = asset_registry->FindByGuid(m_guid); res.is_some()) {
            auto handle = res.unwrap_unchecked();
            if (handle.IsReady()) {
                const auto meta = handle.GetMeta();
                LOG_OK("Asset {} selected", meta->name);
                m_editor.GetViewer().OpenTab(meta->type, m_guid);

                m_editor.SetSelectedAsset(std::move(handle));
                return true;
            }
        }
        return false;
    }

protected:
    const Guid m_guid;
};

// @TODO: SaveDocumentCommand
#if 0
/// SaveProjectCommand
void SaveProjectCommand::Execute(Scene& p_scene) {
    unused(p_scene);
    LOG_WARN("TODO: implement SaveProjectCommand");
    std::string scene;
    if (scene.empty()) {
        return;
    }

    std::filesystem::path path{ scene.empty() ? "untitled.scene" : scene.c_str() };
    if (m_openDialog || scene.empty()) {
// @TODO: implement
#if USING(PLATFORM_WINDOWS)
        if (!os::OpenSaveDialog(path)) {
            return;
        }
#else
        LOG_WARN("OpenSaveDialog not implemented");
#endif
    }

    auto path_string = path.string();

    [[maybe_unused]] const auto extension = StringUtils::Extension(path_string);
    LOG_OK("scene saved to '{}'", path.string());
}
#endif

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

class AddComponentCommand : public EditService::ICommand {
public:
    AddComponentCommand(EditorState& p_editor,
                        SceneId p_scene_id,
                        ComponentName p_type,
                        ecs::Entity p_target)
        : EditService::ICommand(p_editor, p_scene_id)
        , m_type(p_type)
        , m_target(p_target) {}

    bool Undo() override {
        return false;
    }

    bool Redo() override {
        Scene* scene = ResolveScene();
        if (!scene) return false;

        DEV_ASSERT(m_target.IsValid());
        switch (m_type) {
#define COMPONENT_DECL(NAME)                      \
    case ComponentName::NAME: {                   \
        scene->Create<NAME##Component>(m_target); \
    } break;
            COMPONENT_LIST
#undef COMPONENT_DECL

            default: {
                CRASH_NOW();
            } break;
        }
        return true;
    }

protected:
    ComponentName m_type;
    ecs::Entity m_target;
};

EditService::EditService(EditorState& p_editor)
    : m_editor(p_editor) {}

void EditService::CommandInspectAsset(const Guid& p_guid) {
    m_pending_commands.emplace_back(std::make_unique<OpenDocumentCommand>(
        m_editor,
        SceneId{},
        p_guid));
}

void EditService::CommandCreateObject(SceneId p_scene_id,
                                      EntityType p_type,
                                      ecs::Entity p_parent) {
    m_pending_commands.emplace_back(std::make_unique<CreateObjectCommand>(
        m_editor,
        p_scene_id,
        p_type,
        p_parent));
}

void EditService::CommandAddComponent(SceneId p_scene_id,
                                      ComponentName p_type,
                                      ecs::Entity p_target) {
    m_pending_commands.emplace_back(std::make_unique<AddComponentCommand>(
        m_editor,
        p_scene_id,
        p_type,
        p_target));
}

void EditService::CommandDeleteObject(SceneId p_scene_id,
                                      ecs::Entity p_target) {
    m_pending_commands.emplace_back(std::make_unique<DeleteObjectCommand>(
        m_editor,
        p_scene_id,
        p_target));
}

void EditService::CommandCloneObject(SceneId p_scene_id,
                                     ecs::Entity p_target) {
    m_pending_commands.emplace_back(std::make_unique<CloneObjectCommand>(
        m_editor,
        p_scene_id,
        p_target));
}

void EditService::Flush() {
    CAVE_PROFILE_EVENT();

    // @TODO: submit to undo queue
    while (!m_pending_commands.empty()) {
        ICommand* task = m_pending_commands.front().get();
        task->Redo();
        m_pending_commands.pop_front();
    }

    // auto& undo_stack = I
    //     task->Redo();
    //     m_pending_commands.pop_front();
    // }
}

}  // namespace cave
