#pragma once
#include <engine/assets/guid.h>
#include <engine/ecs/entity.h>
#include <engine/math/geomath.h>

#include "editor/Enums.h"
#include "editor/undo_redo/UndoCommand.h"

namespace cave {

class EditorState;
class Scene;

class EditorCommandBase {
public:
    EditorCommandBase() {}

    virtual ~EditorCommandBase() = default;

    virtual void Execute(Scene& p_scene) = 0;

protected:
    EditorState* m_editor{ nullptr };

    friend class EditorState;
};

// @TODO: change to memento
class EditorUndoCommandBase : public EditorCommandBase, public UndoCommand {
public:
    EditorUndoCommandBase()
        : EditorCommandBase() {}

    void Execute(Scene&) final { Redo(); }
};

class EditorInspectAssetCommand : public EditorCommandBase {
public:
    EditorInspectAssetCommand(const Guid& p_guid)
        : m_guid(p_guid) {}

    void Execute(Scene& p_scene) override;

protected:
    const Guid m_guid;
};

class EditorCommandAddEntity : public EditorCommandBase {
public:
    EditorCommandAddEntity(EntityType p_entity_type)
        : m_entityType(p_entity_type) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    EntityType m_entityType;
    ecs::Entity m_parent;
    ecs::Entity m_entity;

    friend class EditorState;
};

class EditorCommandAddComponent : public EditorCommandBase {
public:
    EditorCommandAddComponent(ComponentName p_component_type)
        : m_componentType(p_component_type) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    ComponentName m_componentType;
    ecs::Entity target;

    friend class EditorState;
};

class EditorCommandRemoveEntity : public EditorCommandBase {
public:
    EditorCommandRemoveEntity(ecs::Entity p_target)
        : m_target(p_target) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    ecs::Entity m_target;

    friend class EditorState;
};

class EditorCommandDuplicateEntity : public EditorCommandBase {
public:
    EditorCommandDuplicateEntity(ecs::Entity p_target)
        : m_target(p_target) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    ecs::Entity m_target;

    friend class EditorState;
};

class SaveProjectCommand : public EditorCommandBase {
public:
    SaveProjectCommand(bool p_open_dialog)
        : m_openDialog(p_open_dialog) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    bool m_openDialog;
};

}  // namespace cave
