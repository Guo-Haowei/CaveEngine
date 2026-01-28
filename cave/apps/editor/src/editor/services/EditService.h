#pragma once
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneId.h"

// @TODO: move it to public

#include "editor/undo_redo/UndoCommand.h"

namespace cave {

class EditorState;
class Guid;
class UndoStack;

// @TODO: refactor this part
enum class EntityType : uint8_t;
enum class ComponentName : uint8_t;

class EditService {
public:
    class ICommand : public UndoCommand {
    public:
        ICommand(EditorState& p_editor,
                 SceneId p_scene_id);

        virtual ~ICommand() = default;

        Scene* ResolveScene();

        SceneId GetSceneId() const { return m_scene_id; }

    protected:
        EditorState& m_editor;
        SceneId m_scene_id;
    };

    EditService(EditorState& p_editor);

    void CommandCreateObject(SceneId p_scene_id,
                             EntityType p_type,
                             ecs::Entity p_parent);

    void CommandAddComponent(SceneId p_scene_id,
                             ComponentName p_type,
                             ecs::Entity p_target);

    void CommandDeleteObject(SceneId p_scene_id,
                             ecs::Entity p_target);

    void CommandCloneObject(SceneId p_scene_id,
                            ecs::Entity p_target);

    void Flush();

#if 0
    bool IsDirty(SceneId);

    void MarkSaved(SceneId);

    uint64_t GetRevision(SceneId);

    void ClearHistory(SceneId);
#endif

private:
    EditorState& m_editor;
    std::list<std::unique_ptr<ICommand>> m_pending_commands;

    // std::unordered_map<SceneId, std::unique_ptr<UndoStack>> m_stacks;
    //  Undo stack per SceneId
};

}  // namespace cave
