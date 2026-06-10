#include "Move.h"

#include <cassert>
#include <format>

namespace chess::core {

static_assert(sizeof(Move) == sizeof(uint16_t));

Move::Move() noexcept {
    uint16_t* val = reinterpret_cast<uint16_t*>(this);
    *val = 0;
}

Move::Move(Square from, Square to, MoveType type, PieceType promo) noexcept
    : from_(from.index())
    , to_(to.index())
    , flag_(std::to_underlying(type)) {
    switch (promo) {
        case PieceType::Knight: {
            promo_ = 0;
        } break;
        case PieceType::Bishop: {
            promo_ = 1;
        } break;
        case PieceType::Rook: {
            promo_ = 2;
        } break;
        // case PieceType::Queen:
        default: {
            promo_ = 3;
        } break;
    }
}

cave::Option<PieceType> Move::promo() const {
    if (type() != MoveType::Promotion) return cave::None();
    switch (promo_) {
        case 0:
            return cave::Some(PieceType::Knight);
        case 1:
            return cave::Some(PieceType::Bishop);
        case 2:
            return cave::Some(PieceType::Rook);
        case 3:
        default:
            return cave::Some(PieceType::Queen);
    }
}

std::string Move::uci() const {
    return std::format("{}{}", from().uci(), to().uci());
}

void MoveList::addMove(Move move) {
    assert(size_ < moves_.size());
    moves_[size_++] = move;
}

void MoveList::clear() {
    size_ = 0;
}

}  // namespace chess::core
