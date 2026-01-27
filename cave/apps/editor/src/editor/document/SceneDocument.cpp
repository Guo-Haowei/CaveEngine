#include "SceneDocument.h"

#include "engine/private/runtime/framework/AssetRegistry.h"
#include "engine/private/runtime/scene/ISceneRegistry.h"
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

SceneDocument::SceneDocument(IApplication& p_app, const Guid& p_guid)
    : DocumentBase(p_app, p_guid) {

    auto scene = std::make_unique<Scene>();
    scene->Copy(*m_handle.Get<Scene>());

    m_preview_scene = m_scene_reg.Register(std::move(scene));
}

bool SceneDocument::Save() {
    return false;
}

bool SceneDocument::SaveAs(std::string_view p_new_path) {
    unused(p_new_path);
    return false;
}

#if 0
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
#endif

}  // namespace cave
