#include "cave/core/error/Error.h"

namespace cave {

template<>
std::string InternalError<ErrorCode>::ToString() const {
    auto ret = std::format("Error \"{}\" occured, \"{}\".", ErrorToString(value_), message_);

    bool first = true;
    for (const ErrorFrame& frame : frames_) {
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
