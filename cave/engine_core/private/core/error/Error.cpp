#include "cave/core/error/Error.h"

#include <filesystem>
// @TODO: use string utils
inline std::string FileNameFromPath(std::string_view path) {
    return std::filesystem::path(path).filename().string();
}

namespace cave {

std::string ToString(const Error& error) {
    auto ret = std::format("Error \"{}\" occured, \"{}\".",
                           ErrorToString(error.value()),
                           error.message());

    bool first = true;
    for (const ErrorFrame& frame : error.frames()) {
        auto stack_trace = std::format("\n    {}at {} (...{}:{})",
                                       first ? "" : "propagated ",
                                       frame.func,
                                       FileNameFromPath(frame.filepath),
                                       frame.line);
        ret.append(stack_trace);
        first = false;
    }

    return ret;
}

}  // namespace cave
