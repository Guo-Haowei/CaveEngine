#include "cave/core/error/Error.h"

namespace cave {

std::string ToString(const Error& error) {
    auto ret = std::format("Error \"{}\" occured, \"{}\".",
                           ErrorToString(error.value()),
                           error.message());

    bool first = true;
    for (const ErrorFrame& frame : error.frames()) {
        auto stack_trace = std::format("\n    {}at {} ({}:{})",
                                       first ? "" : "propagated ",
                                       frame.func,
                                       frame.filepath,
                                       frame.line);
        ret.append(stack_trace);
        first = false;
    }

    return ret;
}

}  // namespace cave
