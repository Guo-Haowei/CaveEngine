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

    Option<std::string_view> Prev();
    Option<std::string_view> Next();

    void ResetNav() { m_index = static_cast<int>(m_history.size()) - 1; }

private:
    IApplication& m_app;
    CommandRegistry& m_reg;
    // @TODO: add draft to history
    std::vector<std::string> m_history;
    int m_index;
};

}  // namespace cave
