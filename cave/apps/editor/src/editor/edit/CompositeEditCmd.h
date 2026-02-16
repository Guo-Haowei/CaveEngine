#pragma once
#include "EditCmdBase.h"

namespace cave {

class CompositeEditCmd final : public IEditCmd {
public:
    const char* Label() const override { return "CompositeEditCmd"; }

    void AddCommand(std::unique_ptr<IEditCmd>&& p_cmd);

    bool Do(IDocument& p_doc) override;
    bool Undo(IDocument& p_doc) override;

    bool CanCoalesceWith(const IEditCmd*) const override { return false; }
    void CoalesceFrom(std::unique_ptr<IEditCmd>) override {}

private:
	std::vector<std::unique_ptr<IEditCmd>> m_child;
};

}  // namespace cave
