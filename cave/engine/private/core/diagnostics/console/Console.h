#pragma once
#include "cave/core/diagnostics/CommandRegistry.h"

namespace cave {

struct EngineServices;

class Console {
public:
    explicit Console(EngineServices& services) noexcept;

    void submitLine(std::string_view line);

    void findByPrefix(std::string_view prefix, Vector<std::string_view>& out) const {
        m_reg.findByPrefix(prefix, out);
    }

    Option<std::string_view> prev();
    Option<std::string_view> next();

    void resetNav() { m_index = static_cast<int>(m_history.size()); }

private:
    EngineServices& m_services;
    CommandRegistry& m_reg;
    // @TODO: add draft to history
    Vector<String> m_history;
    int m_index;
};

}  // namespace cave
