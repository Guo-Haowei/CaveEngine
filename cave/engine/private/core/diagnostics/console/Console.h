#pragma once
#include "CommandRegistry.h"

namespace cave {

class Console {
public:
    explicit Console(CommandRegistry& p_reg) noexcept
        : m_reg(p_reg) {}

    void SubmitLine(std::string_view p_line);

    void FindByPrefix(std::string_view p_prefix, std::vector<std::string_view>& p_out) const {
        m_reg.FindByPrefix(p_prefix, p_out);
    }

    // @TODO: auto complete
    // @TODO: history

private:
    CommandRegistry& m_reg;
};

}  // namespace cave
