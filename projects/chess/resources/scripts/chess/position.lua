-- file: position.lua
local move = require('@res://scripts/chess/move.lua')

local Position = {}
Position.__index = Position

-- Piece indices for arrays
local P = {
    WP = 1, WN = 2, WB = 3, WR = 4, WQ = 5, WK = 6,
    BP = 7, BN = 8, BB = 9, BR = 10, BQ = 11, BK = 12,
}

local PIECE_CHAR = {
    [P.WP] = 'P', [P.WN] = 'N', [P.WB] = 'B', [P.WR] = 'R', [P.WQ] = 'Q', [P.WK] = 'K',
    [P.BP] = 'p', [P.BN] = 'n', [P.BB] = 'b', [P.BR] = 'r', [P.BQ] = 'q', [P.BK] = 'k',
}

local CHAR_TO_PIECE = {}
for k, v in pairs(PIECE_CHAR) do
    CHAR_TO_PIECE[v] = k
end

local function u64(x)
    -- placeholder: in Lua 5.3+ you can use integer arithmetic.
    -- if you later need strict u64 behavior, wrap with bit ops.
    return x
end

local function bb_mask(sq)
    return u64(1) << sq
end

local function clone_array(a)
    local b = {}
    for i = 1, #a do
        b[i] = a[i]
    end
    return b
end

function Position.new()
    local self = setmetatable({}, Position)

    -- 12 piece bitboards
    self.bb = {}
    for i = P.WP, P.BK do
        self.bb[i] = u64(0)
    end

    self.side_to_move = 'w' -- 'w' or 'b'
    self.castling = 'KQkq'
    self.ep = nil
    self.halfmove = 0
    self.fullmove = 1
    self._history = {}

    return self
end

function Position:occ_white()
    return self.bb[P.WP] | self.bb[P.WN] | self.bb[P.WB] | self.bb[P.WR] | self.bb[P.WQ] | self.bb[P.WK]
end

function Position:occ_black()
    return self.bb[P.BP] | self.bb[P.BN] | self.bb[P.BB] | self.bb[P.BR] | self.bb[P.BQ] | self.bb[P.BK]
end

function Position:occ_all()
    return self:occ_white() | self:occ_black()
end

function Position:piece_at(sq)
    local m = bb_mask(sq)
    for i = 1, 12 do
        if (self.bb[i] & m) ~= 0 then
            return i
        end
    end
    return nil
end

function Position:remove_piece_at(sq)
    local m = ~bb_mask(sq)
    for i = 1, 12 do
        self.bb[i] = self.bb[i] & m
    end
end

function Position:set_piece_at(sq, piece_index)
    self:remove_piece_at(sq)
    self.bb[piece_index] = self.bb[piece_index] | bb_mask(sq)
end

function Position:_push_snapshot()
    table.insert(self._history, {
        bb = clone_array(self.bb),
        turn = self.turn,
        castling = self.castling,
        ep = self.ep,
        halfmove = self.halfmove,
        fullmove = self.fullmove,
    })
end

function Position:pop()
    local snap = table.remove(self._history)
    if not snap then
        return false, 'no history'
    end
    self.bb = snap.bb
    self.turn = snap.turn
    self.castling = snap.castling
    self.ep = snap.ep
    self.halfmove = snap.halfmove
    self.fullmove = snap.fullmove
    return true
end

-- Blind make-move: applies from->to, captures whatever is on 'to'.
-- Handles promotion (replaces moving piece type) but does NOT enforce promo correctness.
-- Clears ep by default. Does not handle castling rook move or en-passant capture yet.
function Position:push(mv)
    if type(mv) == 'string' then
        local parsed, err = move.from_uci(mv)
        if not parsed then
            return false, err
        end
        mv = parsed
    end
    if type(mv) ~= 'table' then
        return false, 'move must be uci string or move table'
    end

    self:_push_snapshot()

    local from_sq = mv.from
    local to_sq = mv.to

    local moving_piece = self:piece_at(from_sq)
    if not moving_piece then
        -- no piece at from; still keep snapshot for debugging? undo anyway
        self:pop()
        return false, 'no piece on from-square'
    end

    -- capture: remove any piece on destination
    self:remove_piece_at(to_sq)

    -- move the piece
    self:remove_piece_at(from_sq)
    self.bb[moving_piece] = self.bb[moving_piece] | bb_mask(to_sq)

    -- promotion: replace pawn with chosen piece (color inferred by moving_piece)
    if mv.promo then
        local promo = mv.promo
        local is_white = (moving_piece <= P.WK)
        local new_piece

        if promo == 'q' then new_piece = is_white and P.WQ or P.BQ end
        if promo == 'r' then new_piece = is_white and P.WR or P.BR end
        if promo == 'b' then new_piece = is_white and P.WB or P.BB end
        if promo == 'n' then new_piece = is_white and P.WN or P.BN end

        if new_piece then
            -- remove whatever we just placed and set promoted
            self:remove_piece_at(to_sq)
            self.bb[new_piece] = self.bb[new_piece] | bb_mask(to_sq)
        end
    end

    -- TODO later:
    -- - en-passant capture
    -- - double pawn push setting ep
    -- - castling rook move + castling rights update
    -- - halfmove/fullmove update rules
    -- - legality validation

    self.ep = nil

    -- switch side
    if self.turn == 'w' then
        self.turn = 'b'
    else
        self.turn = 'w'
        self.fullmove = self.fullmove + 1
    end

    return true
end

-- FEN parsing (board + turn + castling + ep + halfmove + fullmove)
function Position:load_fen(fen)
    if type(fen) ~= 'string' then
        return false, 'fen must be string'
    end

    local parts = {}
    for part in fen:gmatch('%S+') do
        table.insert(parts, part)
    end
    if #parts < 4 then
        return false, 'fen must have at least 4 fields'
    end

    local board_part = parts[1]
    local turn_part = parts[2]
    local castling_part = parts[3]
    local ep_part = parts[4]
    local halfmove_part = parts[5]
    local fullmove_part = parts[6]

    -- clear
    for i = 1, 12 do
        self.bb[i] = u64(0)
    end

    -- parse board ranks 8..1
    local rank = 7
    local file = 0
    for ch in board_part:gmatch('.') do
        if ch == '/' then
            rank = rank - 1
            file = 0
        elseif ch:match('%d') then
            file = file + tonumber(ch)
        else
            local piece_index = CHAR_TO_PIECE[ch]
            if not piece_index then
                return false, 'invalid piece char in fen: ' .. ch
            end
            local sq = rank * 8 + file
            self.bb[piece_index] = self.bb[piece_index] | bb_mask(sq)
            file = file + 1
        end
    end

    self.turn = (turn_part == 'b') and 'b' or 'w'
    self.castling = (castling_part == '-') and '' or castling_part

    if ep_part == '-' then
        self.ep = nil
    else
        local ep_sq, err = move.square_to_index(ep_part)
        if not ep_sq then
            return false, err
        end
        self.ep = ep_sq
    end

    self.halfmove = tonumber(halfmove_part) or 0
    self.fullmove = tonumber(fullmove_part) or 1

    -- reset history
    self._history = {}

    return true
end

local function emit_rank(pos, rank)
    local out = {}
    local empty = 0

    for file = 0, 7 do
        local sq = rank * 8 + file
        local p = pos:piece_at(sq)
        if not p then
            empty = empty + 1
        else
            if empty > 0 then
                table.insert(out, tostring(empty))
                empty = 0
            end
            table.insert(out, PIECE_CHAR[p])
        end
    end

    if empty > 0 then
        table.insert(out, tostring(empty))
    end
    return table.concat(out)
end

function Position:fen()
    local ranks = {}
    for r = 7, 0, -1 do
        table.insert(ranks, emit_rank(self, r))
    end

    local board = table.concat(ranks, '/')
    local turn = self.turn
    local castling = (self.castling == '' and '-') or self.castling
    local ep = '-'
    if self.ep ~= nil then
        ep = move.index_to_square(self.ep)
    end

    return string.format('%s %s %s %s %d %d', board, turn, castling, ep, self.halfmove, self.fullmove)
end

-- For UI: 64-array of chars like 'P','p', or nil
function Position:board_array()
    local a = {}
    for sq = 0, 63 do
        local p = self:piece_at(sq)
        a[sq] = p and PIECE_CHAR[p] or nil
    end
    return a
end

Position.PIECE = P

return Position
