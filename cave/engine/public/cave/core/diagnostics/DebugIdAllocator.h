// =============================================================================
// File: engine/public/cave/core/diagnostics/DebugIdAllocator.h
// =============================================================================
#pragma once
#include "cave/core/ids/DebugId.h"

namespace cave::detail {

constexpr std::string_view TrimPrefix(std::string_view s, std::string_view prefix) {
    if (s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix) {
        return s.substr(prefix.size());
    }
    return s;
}

constexpr std::string_view StripClassStruct(std::string_view s) {
    s = TrimPrefix(s, "class ");
    s = TrimPrefix(s, "struct ");
    s = TrimPrefix(s, "enum ");
    return s;
}

constexpr std::string_view StripNamespace(std::string_view s) {
    // Keep only the last component after "::"
    auto pos = s.rfind("::");
    return (pos == std::string_view::npos) ? s : s.substr(pos + 2);
}

#if defined(_MSC_VER)
constexpr std::string_view ExtractTypeName(std::string_view sig) {
    // e.g. "class std::basic_string_view<char,struct std::char_traits<char> > __cdecl cave::TypeName<class cave::Foo>(void)"
    auto start = sig.find('<');
    auto end = sig.rfind('>');
    if (start == std::string_view::npos || end == std::string_view::npos || end <= start) {
        return {};
    }
    return sig.substr(start + 1, end - (start + 1));
}
#elif defined(__clang__) || defined(__GNUC__)
constexpr std::string_view ExtractTypeName(std::string_view sig) {
    // e.g. "constexpr std::string_view cave::TypeName() [T = cave::Foo]"
    auto start = sig.find("T = ");
    auto end = sig.rfind(']');
    if (start == std::string_view::npos || end == std::string_view::npos || end <= start) {
        return {};
    }
    start += 4;
    return sig.substr(start, end - start);
}
#else
constexpr std::string_view ExtractTypeName(std::string_view) { return {}; }
#endif

uint64_t GenUID();

}  // namespace cave::detail

namespace cave {

template<typename T>
constexpr std::string_view TypeNameFull() {
#if defined(_MSC_VER)
    return detail::ExtractTypeName(__FUNCSIG__);
#else
    return detail::ExtractTypeName(__PRETTY_FUNCTION__);
#endif
}

template<typename T>
constexpr std::string_view TypeNamePretty() {
    auto s = detail::StripClassStruct(TypeNameFull<T>());
    s = detail::StripNamespace(s);
    return s;
}

template<typename T>
static DebugId MakeDebugId(T*) {
    return {
        detail::GenUID(),
        TypeNamePretty<T>(),
    };
};

}  // namespace cave
