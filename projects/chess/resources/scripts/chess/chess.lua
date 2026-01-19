-- file: chess.lua
local Position = require('@res://scripts/chess/position.lua')

local Chess = {}
Chess.__index = Chess

-- local START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
local START_FEN = '8/5k2/3p4/1p1Pp2p/pP2Pp1P/P4P1K/8/8 b - - 99 50'

function Chess.new(opts)
    local self = setmetatable({}, Chess)
    self.pos = Position.new()
    self.pos:load_fen(START_FEN)
    return self
end

function Chess.from_fen(fen)
    local self = setmetatable({}, Chess)
    self.pos = Position.new()
    local ok, err = self.pos:load_fen(fen)
    if not ok then
        return nil, err
    end
    return self
end

function Chess:fen()
    return self.pos:fen()
end

function Chess:turn()
    return self.pos.turn
end

function Chess:push(mv)
    return self.pos:push(mv)
end

function Chess:pop()
    return self.pos:pop()
end

function Chess:board_array()
    return self.pos:board_array()
end

-- Stubs for later
function Chess:legal_moves(opts)
    return {}, 'not implemented'
end

function Chess:is_legal(mv)
    return false, 'not implemented'
end

function Chess:get_piece(file, rank)
    if file < 1 or file > 8 or rank < 1 or rank > 8 then
        return nil
    end

    local sq = (rank - 1) * 8 + (file - 1)
    return self.pos:piece_at(sq)
end

return {
    new = Chess.new,
    from_fen = Chess.from_fen,
    START_FEN = START_FEN,
}
