#pragma once
#include "IEditorItem.h"

#include "editor/EditorState.h"

namespace cave {

IEditorItem::IEditorItem(EditorState& editor)
    : m_engine_services(editor.app().services())
    , m_editor_services(editor.services()) {}

}  // namespace cave
