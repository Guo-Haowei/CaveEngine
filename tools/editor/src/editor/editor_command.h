#pragma once
#include "enums.h"

#include "engine/assets/guid.h"
#include "engine/ecs/entity.h"
#include "engine/math/geomath.h"
#include "editor/undo_redo/undo_command.h"

namespace cave {

class EditorLayer;
class Scene;

class EditorCommandBase {
public:
    EditorCommandBase() {}

    virtual ~EditorCommandBase() = default;

    virtual void Execute(Scene& p_scene) = 0;

protected:
    EditorLayer* m_editor{ nullptr };

    friend class EditorLayer;
};

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

    friend class EditorLayer;
};

class EditorCommandAddComponent : public EditorCommandBase {
public:
    EditorCommandAddComponent(ComponentType p_component_type)
        : m_componentType(p_component_type) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    ComponentType m_componentType;
    ecs::Entity target;

    friend class EditorLayer;
};

class EditorCommandRemoveEntity : public EditorCommandBase {
public:
    EditorCommandRemoveEntity(ecs::Entity p_target)
        : m_target(p_target) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    ecs::Entity m_target;

    friend class EditorLayer;
};

#if 0
class OpenProjectCommand : public EditorCommandBase {
public:
    OpenProjectCommand(bool p_open_dialog) : EditorCommandBase(COMMAND_TYPE_OPEN_PROJECT), m_openDialog(p_open_dialog) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    bool m_openDialog;
};

class SaveProjectCommand : public EditorCommandBase {
public:
    SaveProjectCommand(bool p_open_dialog) : EditorCommandBase(COMMAND_TYPE_SAVE_PROJECT), m_openDialog(p_open_dialog) {}

    virtual void Execute(Scene& p_scene) override;

protected:
    bool m_openDialog;
};
#endif

// @TODO: move it to gizmo
class EntityTransformCommand : public EditorUndoCommandBase {
public:
    EntityTransformCommand(GizmoAction p_action,
                           Scene& p_scene,
                           ecs::Entity p_entity,
                           const Matrix4x4f& p_before,
                           const Matrix4x4f& p_after);

    bool Undo() override;
    bool Redo() override;

    bool MergeCommand(const UndoCommand* p_command) override;

protected:
    GizmoAction m_action;
    Scene& m_scene;
    ecs::Entity m_entity;

    Matrix4x4f m_before;
    Matrix4x4f m_after;
};

}  // namespace cave
