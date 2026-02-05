#pragma once
#include "cave/core/diagnostics/CommandRegistry.h"

namespace cave {

class Console {
public:
    explicit Console(IApplication& p_app) noexcept;

    void SubmitLine(std::string_view p_line);

    void FindByPrefix(std::string_view p_prefix, std::vector<std::string_view>& p_out) const {
        m_reg.FindByPrefix(p_prefix, p_out);
    }

    // @TODO: auto complete
    // @TODO: history

private:
    IApplication& m_app;
    CommandRegistry& m_reg;
};

}  // namespace cave
