#include "editor_command.h"

#include "editor/editor_layer.h"
#include "editor/viewer/viewer.h"

#include "engine/core/os/platform_io.h"
#include "engine/core/string/string_utils.h"
#include "engine/runtime/asset_registry.h"
#include "engine/runtime/common_dvars.h"
#include "engine/runtime/scene_manager_interface.h"
#include "engine/scene/entity_factory.h"

namespace cave {

static std::string GenerateName(std::string_view p_name) {
    static int s_counter = 0;
    return std::format("{}-{}", p_name, ++s_counter);
}

/// EditorInspectAssetCommand
void EditorInspectAssetCommand::Execute(Scene&) {
    auto asset_registry = m_editor->GetApplication()->GetAssetRegistry();
    if (auto res = asset_registry->FindByGuid(m_guid); res.is_some()) {
        auto handle = res.unwrap_unchecked();
        if (handle.IsReady()) {
            const auto meta = handle.GetMeta();
            LOG_OK("Asset {} selected", meta->name);
            m_editor->GetViewer().OpenTab(meta->type, m_guid);

            m_editor->SetSelectedAsset(std::move(handle));
        }
    }
}

/// EditorCommandAddEntity
// @TODO: move this to SceneEditor queue
void EditorCommandAddEntity::Execute(Scene& p_scene) {
    ecs::Entity id;
    switch (m_entityType) {
#define ENTITY_TYPE(NAME, ...)                                                  \
    case EntityType::NAME: {                                                    \
        id = EntityFactory::Create##NAME##Entity(p_scene, GenerateName(#NAME)); \
    } break;
        ENTITY_TYPE_LIST
#undef ENTITY_TYPE
        default:
            LOG_FATAL("Entity type {} not supported", static_cast<int>(m_entityType));
            break;
    }

    if (p_scene.m_root.IsValid()) {
        p_scene.AttachChild(id, m_parent.IsValid() ? m_parent : p_scene.m_root);
    } else {
        p_scene.m_root = id;
    }

    // m_editor->SelectEntity(id);

    ISceneManager::GetSingleton().BumpRevision();
}

/// EditorCommandAddComponent
void EditorCommandAddComponent::Execute(Scene& p_scene) {
    DEV_ASSERT(target.IsValid());
    switch (m_componentType) {
#define COMPONENT_DECL(NAME)                     \
    case ComponentName::NAME: {                  \
        p_scene.Create<NAME##Component>(target); \
    } break;
        COMPONENT_LIST
#undef COMPONENT_DECL

        default: {
            CRASH_NOW();
        } break;
    }
}

/// EditorCommandRemoveEntity
void EditorCommandRemoveEntity::Execute(Scene& p_scene) {
    auto entity = m_target;
    DEV_ASSERT(entity.IsValid());
    p_scene.RemoveEntity(entity);
}

/// SaveProjectCommand
void SaveProjectCommand::Execute(Scene& p_scene) {
    unused(p_scene);

    CRASH_NOW();
    std::string scene;

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

}  // namespace cave
