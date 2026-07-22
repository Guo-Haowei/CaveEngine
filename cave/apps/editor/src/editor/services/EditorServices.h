#pragma once

namespace cave {

class DocumentService;
class DragDropService;
class EditService;
class IconCache;
class PickingService;
class SceneEditService;
class SelectionService;
class ShortcutService;
class ThumbnailService;
class Workspace;

struct EditorServices {
    DocumentService* document_service{};
    DragDropService* drag_drop{};
    EditService* edit_service{};
    IconCache* icon_cache{};
    PickingService* picking_service{};
    SceneEditService* scene_edit{};
    SelectionService* selection_service{};
    ShortcutService* shortcut_service{};
    ThumbnailService* thumbnail_service{};
    Workspace* workspace_service{};

    DocumentService& document() { return *document_service; }
    DragDropService& dragDrop() { return *drag_drop; }
    EditService& edit() { return *edit_service; }
    IconCache& iconCache() { return *icon_cache; }
    PickingService& picking() { return *picking_service; }
    SceneEditService& sceneEdit() { return *scene_edit; }
    SelectionService& selection() { return *selection_service; }
    ShortcutService& shortcut() { return *shortcut_service; }
    ThumbnailService& thumbnail() { return *thumbnail_service; }
    Workspace& workspace() { return *workspace_service; }
};

}  // namespace cave
