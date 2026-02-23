#include "Move.h"

#include <format>

namespace chess::core {

static_assert(sizeof(Move) == sizeof(uint16_t));

Move::Move() noexcept {
    uint16_t* val = reinterpret_cast<uint16_t*>(this);
    *val = 0;
}

Move::Move(Square p_from, Square p_to, MoveType p_type, PieceType p_promotion) noexcept
    : m_from(p_from.Index())
    , m_to(p_to.Index())
    , m_flag(std::to_underlying(p_type)) {
    switch (p_promotion) {
        case PieceType::Knight: {
            m_promo = 0;
        } break;
        case PieceType::Bishop: {
            m_promo = 1;
        } break;
        case PieceType::Rook: {
            m_promo = 2;
        } break;
        // case PieceType::Queen:
        default: {
            m_promo = 3;
        } break;
    }
}

std::string Move::Uci() const {
    return std::format("{}{}", From().ToString(), To().ToString());
}

}  // namespace chess::core
