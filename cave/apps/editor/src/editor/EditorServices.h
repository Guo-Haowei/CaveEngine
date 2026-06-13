#pragma once

namespace cave {

class DocumentService;
class EditService;
class IconCache;
class PickingService;
class SelectionService;
class ShortcutService;
class ThumbnailService;
class Workspace;

struct EditorServices {
    DocumentService* document_{};
    EditService* edit_{};
    IconCache* icon_cache_{};
    PickingService* picking_{};
    SelectionService* selection_{};
    ShortcutService* shortcut_{};
    ThumbnailService* thumbnail_{};
    Workspace* workspace_{};

    DocumentService& document() { return *document_; }
    EditService& edit() { return *edit_; }
    IconCache& iconCache() { return *icon_cache_; }
    PickingService& picking() { return *picking_; }
    SelectionService& selection() { return *selection_; }
    ShortcutService& shortcut() { return *shortcut_; }
    ThumbnailService& thumbnail() { return *thumbnail_; }
    Workspace& workspace() { return *workspace_; }
};

}  // namespace cave
