#include "tile_set_editor.h"

#include <IconsFontAwesome/IconsFontAwesome6.h >

#include "engine/scene/camera_component.h"

#include "editor/document/document.h"

namespace cave {

TileSetEditor::TileSetEditor(EditorLayer& p_editor, Viewer& p_viewer)
    : ViewerTab(p_editor, p_viewer) {

    m_camera = std::make_unique<CameraComponent>();
    ViewerTab::CreateDefaultCamera2D(*m_camera.get());
}

TileSetEditor::~TileSetEditor() = default;

bool TileSetEditor::HandleInput(const InputEvent* p_input_event) {
    unused(p_input_event);
    return false;
}

void TileSetEditor::OnCreate(const Guid& p_guid) {
    unused(p_guid);

    m_document = std::make_unique<Document>(p_guid);
}

void TileSetEditor::OnDestroy() {
}

void TileSetEditor::OnActivate() {
}

void TileSetEditor::DrawMainView(const CameraComponent& p_camera) {
    unused(p_camera);
}

void TileSetEditor::DrawAssetInspector() {
}

Document& TileSetEditor::GetDocument() const {
    return *m_document;
}

const CameraComponent& TileSetEditor::GetActiveCameraInternal() const {
    DEV_ASSERT(m_camera);
    return *m_camera.get();
}

const std::vector<const ToolBarButtonDesc*> TileSetEditor::GetToolBarButtons() const {
    return {};
}

}  // namespace cave
