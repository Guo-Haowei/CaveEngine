#include "DocumentService.h"

#include "SceneDocument.h"

#include "cave/runtime/framework/IApplication.h"

#include "editor/EditorState.h"

namespace cave {

DocumentService::DocumentService(EditorState& p_editor)
    : m_editor(p_editor) {
}

DocId DocumentService::OpenScene(const Guid& p_guid) {
    auto doc = std::make_unique<SceneDocument>(m_editor.GetApp(), p_guid);
    return Base::Create(std::move(doc));
}

DocId DocumentService::Create(const CreateDocDesc& p_desc) {
    unused(p_desc);
    return {};
}

CloseRequestResult DocumentService::Close(DocId p_id) {
    unused(p_id);
    return {};
}

}  // namespace cave
