-- file: chess.lua
local Position = require('@res://scripts/chess/position.lua')
local MoveGen  = require('@res://scripts/chess/move_gen.lua')

local Chess = {}
Chess.__index = Chess

local START_FEN = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'

function Chess.new(opts)
    local self = setmetatable({}, Chess)
    self.pos = Position.new()
    self.pos:load_fen(START_FEN)

    self:update_legal_moves()
    return self
end

function Chess.from_fen(fen)
    local self = setmetatable({}, Chess)
    self.pos = Position.new()
    local ok, err = self.pos:load_fen(fen)
    if not ok then
        return nil, err
    end

    self:update_legal_moves()
    return self
end

function Chess:piece_color(piece)
    if not piece then
        return nil
    end

    if piece >= Position.PIECE.WP and piece <= Position.PIECE.WK then
        return 'w'
    end

    if piece >= Position.PIECE.BP and piece <= Position.PIECE.BK then
        return 'b'
    end

    return nil
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

function Chess:update_legal_moves()
    self.legal_moves = MoveGen.generate_all(self.pos)
    local map = {}
    for i = 1, #self.legal_moves do
        local mv = self.legal_moves[i]
        if map[mv.from] == nil then
            map[mv.from] = {}
        end
        local sub = map[mv.from]

        sub[#sub + 1] = mv
    end
    self.legal_moves_map = map
end

function Chess:legal_moves(opts)
    return self.legal_moves
end

function Chess:legal_moves_from(index)
    return self.legal_moves_map[index] or {}
end

function Chess:can_move(index)
    return self.legal_moves_map[index] ~= nil
end

function Chess:is_legal(mv)
    local sub = self.legal_moves_map[mv.from]
    if not sub then
        return false
    end
    for i = 1, #sub do
        local lm = sub[i]
        if lm.to == mv.to and lm.promo == mv.promo then
            return true
        end
    end
    return false
end

function Chess:get_piece(file, rank)
    if file < 1 or file > 8 or rank < 1 or rank > 8 then
        return nil
    end

    local sq = (rank - 1) * 8 + (file - 1)
    return self.pos:piece_at(sq)
end

function Chess:make_move(mv)
    ok, err = self.pos:push(mv)
    self:update_legal_moves()
    return ok, err
end

return {
    new = Chess.new,
    from_fen = Chess.from_fen,
    START_FEN = START_FEN,
}
