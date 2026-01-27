#include "EditService.h"

#include "engine/private/debugger/profiler.h"

namespace cave {

void EditService::BufferCommand(std::shared_ptr<EditorCommandBase>&& p_command) {
    p_command->m_editor = &m_editor;
    m_command_buffer.emplace_back(std::move(p_command));
}

void EditService::CommandInspectAsset(const Guid& p_guid) {
    auto command = std::make_shared<EditorInspectAssetCommand>(p_guid);
    BufferCommand(command);
}

void EditService::CommandAddComponent(ComponentName p_type, ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandAddComponent>(p_type);
    command->target = p_target;
    BufferCommand(command);
}

void EditService::CommandAddEntity(EntityType p_type, ecs::Entity p_parent) {
    auto command = std::make_shared<EditorCommandAddEntity>(p_type);
    command->m_parent = p_parent;
    BufferCommand(command);
}

void EditService::CommandRemoveEntity(ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandRemoveEntity>(p_target);
    BufferCommand(command);
}

void EditService::CommandDuplicateEntity(ecs::Entity p_target) {
    auto command = std::make_shared<EditorCommandDuplicateEntity>(p_target);
    BufferCommand(command);
}

void EditService::FlushCommand(Scene* p_scene) {
    CAVE_PROFILE_EVENT();

    while (!m_command_buffer.empty()) {
        auto task = m_command_buffer.front();
        m_command_buffer.pop_front();
        task->Execute(*p_scene);
    }
}

}  // namespace cave
