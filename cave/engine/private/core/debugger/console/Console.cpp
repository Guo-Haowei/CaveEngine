#include "Console.h"

namespace cave::debug {

void Console::SubmitLine(std::string_view p_line) {
    LOG("command [{}] received", p_line);
}

}  // namespace cave::debug
