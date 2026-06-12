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
        : value_(value) {
        frames_.emplace_back(filepath, func, line);
    }

    template<typename... Args>
    InternalError(std::string_view filepath,
                  std::string_view function,
                  int line,
                  const T& value,
                  std::format_string<Args...> format,
                  Args&&... args)
        : InternalError(filepath, function, line, value) {
        message_ = std::format(format, std::forward<Args>(args)...);
    }

    InternalError(std::string_view filepath,
                  std::string_view func,
                  int line,
                  InternalError<T>&& other)
        : value_(std::move(other.value_))
        , message_(std::move(other.message_))
        , frames_(std::move(other.frames_)) {
        frames_.push_back(ErrorFrame{
            .filepath = filepath,
            .func = func,
            .line = line,
        });
    }

    const T& value() const { return value_; }
    std::string_view message() const { return message_; }
    std::span<const ErrorFrame> frames() const { return frames_; }

private:
    T value_{};
    std::string message_;
    std::vector<ErrorFrame> frames_;
};

using Error = InternalError<ErrorCode>;

CAVE_CORE_API std::string ToString(const Error& error);

}  // namespace cave
