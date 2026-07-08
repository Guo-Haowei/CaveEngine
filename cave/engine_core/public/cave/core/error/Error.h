// =============================================================================
// File: cave/core/error/Error.h
// =============================================================================
#pragma once
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "cave/core/CoreExport.h"
#include "cave/core/error/ErrorCode.h"

namespace cave {

struct ErrorFrame {
    std::string_view filepath;
    std::string_view func;
    int line;
};

template<typename T>
struct InternalError {
public:
    InternalError(std::string_view filepath,
                  std::string_view func,
                  int line,
                  const T& value)
        : m_value(value) {
        m_frames.emplace_back(filepath, func, line);
    }

    template<typename... Args>
    InternalError(std::string_view filepath,
                  std::string_view function,
                  int line,
                  const T& value,
                  std::format_string<Args...> format,
                  Args&&... args)
        : InternalError(filepath, function, line, value) {
        m_message = std::format(format, std::forward<Args>(args)...);
    }

    InternalError(std::string_view filepath,
                  std::string_view func,
                  int line,
                  InternalError<T>&& other)
        : m_value(std::move(other.m_value))
        , m_message(std::move(other.m_message))
        , m_frames(std::move(other.m_frames)) {
        m_frames.push_back(ErrorFrame{
            .filepath = filepath,
            .func = func,
            .line = line,
        });
    }

    const T& value() const { return m_value; }
    std::string_view message() const { return m_message; }
    std::span<const ErrorFrame> frames() const { return m_frames; }

private:
    T m_value{};
    std::string m_message;
    std::vector<ErrorFrame> m_frames;  // @TODO: small vector optimization
};

using Error = InternalError<ErrorCode>;

CAVE_CORE_API std::string ToString(const Error& error);

}  // namespace cave
