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

    bool canUndo() const override { return !m_undo.empty(); }
    bool canRedo() const override { return !m_redo.empty(); }

    bool undo() override;

    bool redo() override;

    void markSaved() override {
        // Save marker is "undo stack size at time of save".
        // If user undoes/redoes to exactly this size again => not dirty.
        m_saved_undo_size = m_undo.size();
    }

    bool isDirty() const override {
        return m_saved_undo_size != m_undo.size();
    }

    void undoLabels(std::vector<std::string>& out, int max_items) const override;

    void redoLabels(std::vector<std::string>& out, int max_items) const override;

    bool save() override;
    bool saveAs(std::string_view) override;

    Guid guid() const override { return m_guid; }

    SceneId previewScene() const override {
        return m_preview_scene;
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
    void undoLimit(size_t limit) { m_undo_limit = limit; }

    EngineServices& m_engine_services;
    AssetRegistry& m_asset_reg;
    EditorAssetManager& m_asset_mgr;
    SceneRegistry& m_scene_reg;
    SceneId m_preview_scene{};
    Guid m_guid;

private:
    std::deque<std::unique_ptr<IEditCmd>> m_undo;
    std::deque<std::unique_ptr<IEditCmd>> m_redo;

    size_t m_undo_limit = 0;       // 0 = unlimited
    size_t m_saved_undo_size = 0;  // save marker

    uint32_t m_last_coalesce = 0;
};

}  // namespace cave
