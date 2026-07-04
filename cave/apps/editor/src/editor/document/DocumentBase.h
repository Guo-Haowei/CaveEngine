#pragma once
#include "IDocument.h"

#include "editor/edit/IEditCmd.h"

namespace cave {

struct EngineServices;
class AssetRegistry;
class EditorAssetManager;
class SceneRegistry;

class DocumentBase : public IDocument {
public:
    DocumentBase(EngineServices& services, const Guid& guid);

    bool apply(std::unique_ptr<IEditCmd> cmd, uint32_t coalesce) override;

    bool canUndo() const override { return !undo_.empty(); }
    bool canRedo() const override { return !redo_.empty(); }

    bool undo() override;

    bool redo() override;

    void markSaved() override {
        // Save marker is "undo stack size at time of save".
        // If user undoes/redoes to exactly this size again => not dirty.
        saved_undo_size_ = undo_.size();
    }

    bool isDirty() const override {
        return saved_undo_size_ != undo_.size();
    }

    void undoLabels(std::vector<std::string>& out, int max_items) const override;

    void redoLabels(std::vector<std::string>& out, int max_items) const override;

    bool save() override;
    bool saveAs(std::string_view) override;

    Guid guid() const override { return guid_; }

    SceneId previewScene() const override {
        return preview_scene_;
    }

    std::unique_ptr<Scene> createPreviewScene() const override;
    void reloadPreviewScene() override;

private:
    void touchDirtyAfterEdit() {
        // nothing required here beyond marker comparison;
        // kept as a hook in case you later add "modified time", etc.
    }

    void recomputeDirtyAfterHistoryMove() {
        // IsDirty uses marker compare; nothing to recompute.
    }

    void trimUndoIfNeeded();

protected:
    void undoLimit(size_t limit) { undo_limit_ = limit; }

    SceneId preview_scene_{};
    AssetRegistry& asset_reg_;
    EditorAssetManager& asset_mgr_;
    SceneRegistry& scene_reg_;
    Guid guid_;

private:
    std::deque<std::unique_ptr<IEditCmd>> undo_;
    std::deque<std::unique_ptr<IEditCmd>> redo_;

    size_t undo_limit_ = 0;       // 0 = unlimited
    size_t saved_undo_size_ = 0;  // save marker

    uint32_t last_coalesce_ = 0;
};

}  // namespace cave
