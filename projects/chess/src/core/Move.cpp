#include "Move.h"

#include <format>

namespace chess::core {

static_assert(sizeof(Move) == 2);

std::string Move::Uci() const {
    return std::format("{}{}", from.ToString(), to.ToString());
}

}  // namespace chess::core
