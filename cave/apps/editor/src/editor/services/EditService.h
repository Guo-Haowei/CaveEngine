#pragma once
#include "cave/runtime/ecs/Entity.h"
#include "cave/runtime/scene/SceneId.h"

#include "editor/document/DocumentTypes.h"
#include "editor/edit/IEditCmd.h"
// @TODO: move it to public

#include "editor/undo_redo/UndoCommand.h"

namespace cave {

class EditorState;
class Guid;
class UndoStack;

// @TODO: refactor this part
enum class EntityType : uint8_t;

class EditService {
public:
    EditService(EditorState& p_editor);

    void Submit(DocId p_doc_id, std::unique_ptr<IEditCmd> p_cmd);

    void Undo(DocId p_doc_id);
    void Redo(DocId p_doc_id);

    bool CanUndo(DocId p_doc_id);
    bool CanRedo(DocId p_doc_id);

    void FlushPendingCmds();

private:
    IDocument* ResolveDoc(DocId p_doc_id);

    EditorState& m_editor;
    std::unordered_map<DocId, std::vector<std::unique_ptr<IEditCmd>>> m_pending_cmds;

    // @TODO: deprecate
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

    void CommandCreateObject(SceneId p_scene_id,
                             EntityType p_type,
                             ecs::Entity p_parent);

    void CommandDeleteObject(SceneId p_scene_id,
                             ecs::Entity p_target);

    void CommandCloneObject(SceneId p_scene_id,
                            ecs::Entity p_target);

private:
    std::list<std::unique_ptr<ICommand>> m_old_pending_commands;
};

}  // namespace cave
