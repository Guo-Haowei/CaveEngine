#pragma once
#include "engine/private/runtime/scene/Scene.h"

namespace cave {

class EditorState;

class IEditorItem {
public:
    inline static constexpr const char* DRAG_DROP_ENV = "DRAG_DROP_ENV";
    inline static constexpr const char* DRAG_DROP_IMPORT = "DRAG_DROP_IMPORT";

    IEditorItem(EditorState& p_editor)
        : m_editor(p_editor) {}
    virtual ~IEditorItem() = default;

    virtual void OnAttach() {}
    virtual void Update(float p_timestep) = 0;

protected:
    EditorState& m_editor;
};

}  // namespace cave
