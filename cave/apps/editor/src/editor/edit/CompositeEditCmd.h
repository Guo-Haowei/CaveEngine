#pragma once
#include "editor/edit/IEditCmd.h"

namespace cave {

class CompositeEditCmd final : public IEditCmd {
public:
    const char* label() const override { return "CompositeEditCmd"; }

    void AddCommand(Owner<IEditCmd>&& cmd);

    bool apply(IDocument& doc) override;
    bool undo(IDocument& doc) override;

    bool canCoalesceWith(const IEditCmd*) const override { return false; }
    void coalesceFrom(Owner<IEditCmd>) override {}

private:
    Vector<Owner<IEditCmd>> m_child;
};

}  // namespace cave
