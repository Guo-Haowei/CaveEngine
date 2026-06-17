#pragma once
#include "editor/EditorState.h"

namespace cave {

struct EngineServices;
class EditorState;

class IEditorItem {
public:
    inline static constexpr const char* DRAG_DROP_ENV = "DRAG_DROP_ENV";
    inline static constexpr const char* DRAG_DROP_IMPORT = "DRAG_DROP_IMPORT";

    IEditorItem(EditorState& editor)
        : m_editor(editor)
        , app_services_(editor.app().services())
        , editor_services_(editor.services()) {}

    virtual ~IEditorItem() = default;

    virtual void onAttach() {}
    virtual void drawUI() = 0;

protected:
    // @TODO: deperecate m_editor
    EditorState& m_editor;
    EngineServices& app_services_;
    EditorServices& editor_services_;
};

}  // namespace cave
