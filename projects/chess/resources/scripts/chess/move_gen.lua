-- file: move_gen_bb.lua
-- Bitboard-based pseudo-legal move generation using Lua bitwise ops (5.3+).
-- Skips: promotion, castling, en-passant, check/pin legality.
--
-- Squares: 0..63, with a1 = 0 (LSB), h8 = 63.
-- Move format: { from = <0..63>, to = <0..63> }

local MoveGen = {}

local function u64(x)
    return x
end

local function bb_of(sq)
    return u64(1) << sq
end

local function file_of(sq)
    return sq % 8
end

local function rank_of(sq)
    return math.floor(sq / 8)
end

-- Bitboard helpers -----------------------------------------------------------

-- Extract and clear least significant 1-bit.
-- Returns (sq, new_bb). If bb == 0, returns (nil, 0).
local function pop_lsb(bb)
    if bb == 0 then
        return nil, 0
    end

    -- two's complement trick: lsb = bb & -bb
    local lsb = bb & (-bb)

    -- find index of that bit (0..63)
    local sq = 0
    local t = lsb
    while (t & 1) == 0 do
        t = t >> 1
        sq = sq + 1
    end

    bb = bb & (bb - 1)
    return sq, bb
end

local function push_move(out, from_sq, to_sq)
    out[#out + 1] = { from = from_sq, to = to_sq }
end

-- Side / piece helpers -------------------------------------------------------

local function is_white_piece(piece_index)
    return piece_index ~= nil and piece_index <= 6
end

local function is_black_piece(piece_index)
    return piece_index ~= nil and piece_index >= 7
end

local function is_friend_piece(piece_index, side)
    if not piece_index then return false end
    if side == 'w' then return is_white_piece(piece_index) end
    return is_black_piece(piece_index)
end

-- Fast occupancy (bitboards) -------------------------------------------------

local function occ_white(pos)
    local P = pos.PIECE
    return pos.bb[P.WP] | pos.bb[P.WN] | pos.bb[P.WB] | pos.bb[P.WR] | pos.bb[P.WQ] | pos.bb[P.WK]
end

local function occ_black(pos)
    local P = pos.PIECE
    return pos.bb[P.BP] | pos.bb[P.BN] | pos.bb[P.BB] | pos.bb[P.BR] | pos.bb[P.BQ] | pos.bb[P.BK]
end

-- Attack generation (bitwise), then convert to moves -------------------------

-- Masks for file wrap prevention
local FILE_A = u64(0x0101010101010101)
local FILE_B = u64(0x0202020202020202)
local FILE_G = u64(0x4040404040404040)
local FILE_H = u64(0x8080808080808080)
local FILE_AB = FILE_A | FILE_B
local FILE_GH = FILE_G | FILE_H

local NOT_FILE_A = ~FILE_A
local NOT_FILE_H = ~FILE_H
local NOT_FILE_AB = ~FILE_AB
local NOT_FILE_GH = ~FILE_GH

local function knight_attacks(from_bb)
    -- Using shift patterns with file masks (a1=LSB layout)
    local a = 0
    a = a | ((from_bb << 17) & NOT_FILE_A)        -- +17: (file+1, rank+2)
    a = a | ((from_bb << 15) & NOT_FILE_H)        -- +15: (file-1, rank+2)
    a = a | ((from_bb << 10) & NOT_FILE_AB)       -- +10: (file+2, rank+1)
    a = a | ((from_bb << 6)  & NOT_FILE_GH)       --  +6: (file-2, rank+1)

    a = a | ((from_bb >> 17) & NOT_FILE_H)        -- -17
    a = a | ((from_bb >> 15) & NOT_FILE_A)        -- -15
    a = a | ((from_bb >> 10) & NOT_FILE_GH)       -- -10
    a = a | ((from_bb >> 6)  & NOT_FILE_AB)       --  -6
    return a
end

local function king_attacks(from_bb)
    local a = 0
    a = a | ((from_bb << 8))
    a = a | ((from_bb >> 8))
    a = a | ((from_bb << 1) & NOT_FILE_A)
    a = a | ((from_bb >> 1) & NOT_FILE_H)
    a = a | ((from_bb << 9) & NOT_FILE_A)
    a = a | ((from_bb << 7) & NOT_FILE_H)
    a = a | ((from_bb >> 7) & NOT_FILE_A)
    a = a | ((from_bb >> 9) & NOT_FILE_H)
    return a
end

-- Sliding move generation (ray stepping, bitwise occupancy tests) ------------

local function gen_ray(out, from_sq, side, occ_all, occ_own, df, dr)
    local f = file_of(from_sq)
    local r = rank_of(from_sq)

    local nf = f + df
    local nr = r + dr

    while nf >= 0 and nf < 8 and nr >= 0 and nr < 8 do
        local to_sq = nr * 8 + nf
        local to_bb = bb_of(to_sq)

        if (to_bb & occ_own) ~= 0 then
            break
        end

        push_move(out, from_sq, to_sq)

        if (to_bb & occ_all) ~= 0 then
            break
        end

        nf = nf + df
        nr = nr + dr
    end
end

local function gen_bishop(out, from_sq, side, occ_all, occ_own)
    gen_ray(out, from_sq, side, occ_all, occ_own,  1,  1)
    gen_ray(out, from_sq, side, occ_all, occ_own,  1, -1)
    gen_ray(out, from_sq, side, occ_all, occ_own, -1,  1)
    gen_ray(out, from_sq, side, occ_all, occ_own, -1, -1)
end

local function gen_rook(out, from_sq, side, occ_all, occ_own)
    gen_ray(out, from_sq, side, occ_all, occ_own,  1,  0)
    gen_ray(out, from_sq, side, occ_all, occ_own, -1,  0)
    gen_ray(out, from_sq, side, occ_all, occ_own,  0,  1)
    gen_ray(out, from_sq, side, occ_all, occ_own,  0, -1)
end

local function gen_queen(out, from_sq, side, occ_all, occ_own)
    gen_bishop(out, from_sq, side, occ_all, occ_own)
    gen_rook(out, from_sq, side, occ_all, occ_own)
end

-- Pawns (no promotion, no ep) ------------------------------------------------

local function gen_pawns(pos, out, side, occ_all, occ_own, occ_enemy)
    local P = pos.PIECE
    local pawns = (side == 'w') and pos.bb[P.WP] or pos.bb[P.BP]

    local dir = (side == 'w') and 8 or -8
    local start_rank = (side == 'w') and 1 or 6

    local bb = pawns
    while bb ~= 0 do
        local from_sq
        from_sq, bb = pop_lsb(bb)

        local f = file_of(from_sq)
        local r = rank_of(from_sq)

        -- forward one
        local to_sq = from_sq + dir
        if to_sq >= 0 and to_sq < 64 then
            local to_bb = bb_of(to_sq)
            if (to_bb & occ_all) == 0 then
                push_move(out, from_sq, to_sq)

                -- forward two from start (requires both squares empty)
                if r == start_rank then
                    local to_sq2 = from_sq + 2 * dir
                    if to_sq2 >= 0 and to_sq2 < 64 then
                        local to_bb2 = bb_of(to_sq2)
                        if (to_bb2 & occ_all) == 0 then
                            push_move(out, from_sq, to_sq2)
                        end
                    end
                end
            end
        end

        -- captures
        local cap_rank = r + ((side == 'w') and 1 or -1)
        if cap_rank >= 0 and cap_rank < 8 then
            if f > 0 then
                local cap_sq = cap_rank * 8 + (f - 1)
                local cap_bb = bb_of(cap_sq)
                if (cap_bb & occ_enemy) ~= 0 then
                    push_move(out, from_sq, cap_sq)
                end
            end
            if f < 7 then
                local cap_sq = cap_rank * 8 + (f + 1)
                local cap_bb = bb_of(cap_sq)
                if (cap_bb & occ_enemy) ~= 0 then
                    push_move(out, from_sq, cap_sq)
                end
            end
        end
    end
end

-- Knights / Kings via bitwise attack sets -----------------------------------

local function gen_jumpers(pos, out, side, occ_own, piece_bb, attack_fn)
    local bb = piece_bb
    while bb ~= 0 do
        local from_sq
        from_sq, bb = pop_lsb(bb)

        local from_bb = bb_of(from_sq)
        local attacks = attack_fn(from_bb) & (~occ_own)

        local att = attacks
        while att ~= 0 do
            local to_sq
            to_sq, att = pop_lsb(att)
            push_move(out, from_sq, to_sq)
        end
    end
end

-- Public API ----------------------------------------------------------------

function MoveGen.generate_all(pos)
    local out = {}

    local side = pos.turn
    local P = pos.PIECE

    local w_occ = occ_white(pos)
    local b_occ = occ_black(pos)

    local occ_all = w_occ | b_occ
    local occ_own = (side == 'w') and w_occ or b_occ
    local occ_enemy = occ_all & (~occ_own)

    -- pawns (bitboard iteration, bitwise occupancy tests)
    gen_pawns(pos, out, side, occ_all, occ_own, occ_enemy)

    -- knights (bitwise attack set)
    local knights = (side == 'w') and pos.bb[P.WN] or pos.bb[P.BN]
    gen_jumpers(pos, out, side, occ_own, knights, knight_attacks)

    -- bishops (ray stepping, bitwise occupancy tests)
    local bishops = (side == 'w') and pos.bb[P.WB] or pos.bb[P.BB]
    do
        local bb = bishops
        while bb ~= 0 do
            local from_sq
            from_sq, bb = pop_lsb(bb)
            gen_bishop(out, from_sq, side, occ_all, occ_own)
        end
    end

    -- rooks
    local rooks = (side == 'w') and pos.bb[P.WR] or pos.bb[P.BR]
    do
        local bb = rooks
        while bb ~= 0 do
            local from_sq
            from_sq, bb = pop_lsb(bb)
            gen_rook(out, from_sq, side, occ_all, occ_own)
        end
    end

    -- queens
    local queens = (side == 'w') and pos.bb[P.WQ] or pos.bb[P.BQ]
    do
        local bb = queens
        while bb ~= 0 do
            local from_sq
            from_sq, bb = pop_lsb(bb)
            gen_queen(out, from_sq, side, occ_all, occ_own)
        end
    end

    -- king (bitwise attack set)
    local king = (side == 'w') and pos.bb[P.WK] or pos.bb[P.BK]
    gen_jumpers(pos, out, side, occ_own, king, king_attacks)

    return out
end

function MoveGen.generate_from(pos, from_sq)
    local out = {}
    local side = pos.turn
    local P = pos.PIECE

    if from_sq < 0 or from_sq > 63 then
        return out
    end

    local piece_index = pos:piece_at(from_sq)
    if not piece_index then
        return out
    end

    if (side == 'w' and not is_white_piece(piece_index)) or (side == 'b' and not is_black_piece(piece_index)) then
        return out
    end

    local w_occ = occ_white(pos)
    local b_occ = occ_black(pos)
    local occ_all = w_occ | b_occ
    local occ_own = (side == 'w') and w_occ or b_occ
    local occ_enemy = occ_all & (~occ_own)

    if piece_index == P.WP or piece_index == P.BP then
        -- single pawn only: use a temporary bitboard
        local tmp_pos = { bb = {}, PIECE = pos.PIECE }
        tmp_pos.bb[P.WP] = (piece_index == P.WP) and bb_of(from_sq) or 0
        tmp_pos.bb[P.BP] = (piece_index == P.BP) and bb_of(from_sq) or 0
        gen_pawns(tmp_pos, out, side, occ_all, occ_own, occ_enemy)
        return out
    end

    if piece_index == P.WN or piece_index == P.BN then
        gen_jumpers(pos, out, side, occ_own, bb_of(from_sq), knight_attacks)
        return out
    end

    if piece_index == P.WK or piece_index == P.BK then
        gen_jumpers(pos, out, side, occ_own, bb_of(from_sq), king_attacks)
        return out
    end

    if piece_index == P.WB or piece_index == P.BB then
        gen_bishop(out, from_sq, side, occ_all, occ_own)
        return out
    end

    if piece_index == P.WR or piece_index == P.BR then
        gen_rook(out, from_sq, side, occ_all, occ_own)
        return out
    end

    if piece_index == P.WQ or piece_index == P.BQ then
        gen_queen(out, from_sq, side, occ_all, occ_own)
        return out
    end

    return out
end

return MoveGen

