#pragma once
#include <vector>
#include "Square.h"

namespace chess::core {

struct Move {
    Square from;
    Square to;
};

using MoveList = std::vector<Move>;

}  // namespace chess::core

