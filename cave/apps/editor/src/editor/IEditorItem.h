#pragma once

namespace cave {

class EditorState;

struct EditorServices;
struct EngineServices;

class IEditorItem {
public:
    inline static constexpr const char* DRAG_DROP_ENV = "DRAG_DROP_ENV";
    inline static constexpr const char* DRAG_DROP_IMPORT = "DRAG_DROP_IMPORT";

    IEditorItem(EditorState& editor);
    virtual ~IEditorItem() = default;

    virtual void onAttach() {}
    virtual void onDetach() {}

    virtual void drawUI() = 0;

protected:
    EngineServices& m_engine_services;
    EditorServices& m_editor_services;
};

}  // namespace cave
