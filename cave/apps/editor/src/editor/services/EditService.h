#pragma once
#include "editor/services/EditCommand.h"

namespace cave {

class EditorCommandBase;
class EditorState;

class EditService {
public:
    EditService(EditorState& p_editor)
        : m_editor(p_editor) {}

    void BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command);
    void CommandInspectAsset(const Guid& p_guid);
    void CommandAddComponent(ComponentName p_type, ecs::Entity p_target);
    void CommandAddEntity(EntityType p_type, ecs::Entity p_parent);
    void CommandRemoveEntity(ecs::Entity p_target);
    void CommandDuplicateEntity(ecs::Entity p_target);

    void FlushCommand(Scene* p_scene);

private:
    EditorState& m_editor;
    std::list<std::shared_ptr<EditorCommandBase>> m_command_buffer;
};

}  // namespace cave
