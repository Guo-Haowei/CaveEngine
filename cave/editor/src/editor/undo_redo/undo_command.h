#pragma once

namespace cave {

class UndoCommand {
public:
    virtual ~UndoCommand() = default;

    virtual bool Undo() = 0;
    virtual bool Redo() = 0;

    virtual bool MergeCommand(const UndoCommand*) { return false; }
};

}  // namespace cave
