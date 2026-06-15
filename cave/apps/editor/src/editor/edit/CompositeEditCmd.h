#pragma once
#include "editor/edit/IEditCmd.h"

namespace cave {

class CompositeEditCmd final : public IEditCmd {
public:
    const char* label() const override { return "CompositeEditCmd"; }

    void AddCommand(std::unique_ptr<IEditCmd>&& p_cmd);

    bool apply(IDocument& p_doc) override;
    bool undo(IDocument& p_doc) override;

    bool canCoalesceWith(const IEditCmd*) const override { return false; }
    void coalesceFrom(std::unique_ptr<IEditCmd>) override {}

private:
    std::vector<std::unique_ptr<IEditCmd>> m_child;
};

}  // namespace cave
