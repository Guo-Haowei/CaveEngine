#include "Console.h"

namespace cave {

void Console::SubmitLine(std::string_view p_line) {
    LOG("command [{}] received", p_line);
}

}  // namespace cave
