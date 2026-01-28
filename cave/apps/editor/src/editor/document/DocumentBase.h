#pragma once
#include "IDocument.h"

#include "editor/edit/IEditCmd.h"

namespace cave {

class IApplication;

class AssetRegistry;
class ISceneRegistry;

class DocumentBase : public IDocument {
public:
    DocumentBase(IApplication& p_app, const Guid& p_guid);

    bool Apply(std::unique_ptr<IEditCmd> p_cmd, uint32_t p_coalesce) override;

    bool CanUndo() const override { return !m_undo.empty(); }
    bool CanRedo() const override { return !m_redo.empty(); }

    bool Undo() override;

    bool Redo() override;

    void MarkSaved() override {
        // Save marker is "undo stack size at time of save".
        // If user undoes/redoes to exactly this size again => not dirty.
        m_saved_undo_size = m_undo.size();
    }

    bool IsDirty() const override {
        return m_saved_undo_size != m_undo.size();
    }

    void GetUndoLabels(std::vector<std::string>& p_out, int p_max_items) const override;

    void GetRedoLabels(std::vector<std::string>& p_out, int p_max_items) const override;

    bool Save() { return false; }
    bool SaveAs(std::string_view) { return false; }

private:
    void TouchDirtyAfterEdit() {
        // nothing required here beyond marker comparison;
        // kept as a hook in case you later add "modified time", etc.
    }

    void RecomputeDirtyAfterHistoryMove() {
        // IsDirty uses marker compare; nothing to recompute.
    }

    void TrimUndoIfNeeded();

protected:
    void SetUndoLimit(size_t limit) { m_undo_limit = limit; }

    AssetRegistry& m_asset_reg;
    ISceneRegistry& m_scene_reg;
    Guid m_guid;

private:
    std::deque<std::unique_ptr<IEditCmd>> m_undo;
    std::deque<std::unique_ptr<IEditCmd>> m_redo;

    size_t m_undo_limit = 0;       // 0 = unlimited
    size_t m_saved_undo_size = 0;  // save marker

    uint32_t m_last_coalesce = 0;
};

}  // namespace cave
