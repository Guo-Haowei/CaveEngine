#pragma once
#include "DocId.h"

namespace cave {

struct IEditCommand;

struct DocInfo {
    DocId id{};
    std::string title;  // "MyScene.scene"
    std::string path;   // on disk (can be empty for new/untitled)
    bool is_dirty{ false };
};

class IDocument {
public:
    virtual ~IDocument() = default;

    virtual DocInfo GetInfo() const = 0;

    // --- Editing (commands mutate document state) ---
    // Apply pushes onto undo stack; clears redo stack.
    // Returns false if rejected (invalid command for current doc state).
    virtual bool Apply(std::unique_ptr<IEditCommand> p_cmd,
                       uint32_t p_coalesce) = 0;

    virtual bool CanUndo() const = 0;
    virtual bool CanRedo() const = 0;

    virtual bool Undo() = 0;
    virtual bool Redo() = 0;

    // --- Dirty tracking ---
    // Marks current state as "saved" (typically after successful Save()).
    virtual void MarkSaved() = 0;
    virtual bool IsDirty() const = 0;

    virtual bool Save() = 0;
    virtual bool SaveAs(std::string_view p_new_path) = 0;

    //// --- Optional: for UI ---
    virtual void GetUndoLabels(std::vector<std::string>& p_out, int p_max_items) const = 0;
    virtual void GetRedoLabels(std::vector<std::string>& p_out, int p_max_items) const = 0;
};

}  // namespace cave
