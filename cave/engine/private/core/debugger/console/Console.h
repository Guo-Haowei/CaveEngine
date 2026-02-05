#pragma once
#include "CommandRegistry.h"

namespace cave::debug {

class Console {
public:
    explicit Console(CommandRegistry& p_reg) noexcept
        : m_reg(p_reg) {}

    void SubmitLine(std::string_view p_line);

    // @TODO: auto complete
    // @TODO: history

private:
    CommandRegistry& m_reg;
};

}  // namespace cave::debug
