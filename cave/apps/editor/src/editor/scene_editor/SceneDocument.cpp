#include "SceneDocument.h"

#include "engine/private/runtime/scene/SceneManager.h"
#include "engine/private/runtime/scene/Scene.h"
#include "editor/undo_redo/UndoStack.h"

namespace cave {

class TransformCommand : public UndoCommand {
public:
    TransformCommand(const Handle<Scene>& p_handle,
                     ecs::Entity p_entity,
                     const Matrix4x4f& p_before,
                     const Matrix4x4f& p_after);

    bool Undo() override;
    bool Redo() override;

    bool MergeCommand(const UndoCommand* p_command) override;

protected:
    Handle<Scene> m_handle;
    ecs::Entity m_entity;

    Matrix4x4f m_before;
    Matrix4x4f m_after;
};

TransformCommand::TransformCommand(const Handle<Scene>& p_handle,
                                   ecs::Entity p_entity,
                                   const Matrix4x4f& p_before,
                                   const Matrix4x4f& p_after)
    : m_handle(p_handle)
    , m_entity(p_entity)
    , m_before(p_before)
    , m_after(p_after) {
}

bool TransformCommand::Undo() {
    if (Scene* scene = m_handle.Get(); scene) {
        TransformComponent* transform = scene->GetComponent<TransformComponent>(m_entity);
        if (DEV_VERIFY(transform)) {
            transform->SetLocalTransform(m_before);
            return true;
        }
    }
    return false;
}

bool TransformCommand::Redo() {
    if (Scene* scene = m_handle.Get(); scene) {
        TransformComponent* transform = scene->GetComponent<TransformComponent>(m_entity);
        if (DEV_VERIFY(transform)) {
            transform->SetLocalTransform(m_after);
            return true;
        }
    }
    return false;
}

bool TransformCommand::MergeCommand(const UndoCommand* p_command) {
    auto command = dynamic_cast<const TransformCommand*>(p_command);
    if (!command) {
        return false;
    }

    if (command->m_entity != m_entity) {
        return false;
    }

    if (m_handle.GetGuid() != command->m_handle.GetGuid()) {
        return false;
    }

    m_after = command->m_after;
    return true;
}

SceneDocument::SceneDocument(const Guid& p_guid, SceneManager& p_scene_manager)
    : Document(p_guid)
    , m_scene_manager(p_scene_manager) {

    m_asset_scene = m_handle.Wait<Scene>();

    auto scene = std::make_unique<Scene>();
    scene->Copy(*m_asset_scene);

    SceneDesc desc;
#if USING(DEBUG_BUILD)
    desc.debug_name = "SceneDocument";
#endif

    m_scene_id = m_scene_manager.Register(desc, std::move(scene));
}

void SceneDocument::RequestMove(ecs::Entity p_entity,
                                const Matrix4x4f& p_before,
                                const Matrix4x4f& p_after,
                                bool p_execute) {
    Handle<Scene> handle = Handle<Scene>(m_handle);

    auto command = std::make_unique<TransformCommand>(
        handle,
        p_entity,
        p_before,
        p_after);

    if (p_execute) {
        command->Redo();
    }

    m_undo_stack->Submit(std::move(command));
}

bool SceneDocument::Save() {
    DEV_ASSERT(0);
    return false;
}

}  // namespace cave
