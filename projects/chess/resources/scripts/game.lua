-- file: game.lua

local EMPTY = '.'
local WP = 'P'
local WN = 'N'
local WB = 'B'
local WR = 'R'
local WQ = 'Q'
local WK = 'K'
local BP = 'p'
local BN = 'n'
local BB = 'b'
local BR = 'r'
local BQ = 'q'
local BK = 'k'

-- board
local Board = {}
Board.__index = Board

local function fen_to_grid(fen)
    assert(type(fen) == 'string', 'FEN must be a string')

    local placement = fen:match('^%s*([^%s]+)')
    assert(placement, 'Invalid FEN: missing placement field')

    local grid = {}
    local rank = 8
    local file = 1

    -- Pre-create rows (rank 8..1 mapped to grid[8]..grid[1])
    for r = 1, 8 do
        grid[r] = {}
    end

    for r = 1, 8 do
        grid[r] = {}
    end

    for i = 1, #placement do
        local ch = placement:sub(i, i)

        if ch == '/' then
            assert(file == 9, ('Invalid FEN: rank %d does not have 8 files'):format(rank))
            rank = rank - 1
            assert(rank >= 1, 'Invalid FEN: too many ranks')
            file = 1

        elseif ch:match('%d') then
            local n = tonumber(ch)
            assert(n >= 1 and n <= 8, 'Invalid FEN: digit out of range')

            for _ = 1, n do
                assert(file <= 8, 'Invalid FEN: too many files in rank')
                grid[rank][file] = EMPTY
                file = file + 1
            end

        else
            -- piece letter
            assert(file <= 8, 'Invalid FEN: too many files in rank')
            assert(('PNBRQKpnbrqk'):find(ch, 1, true), ('Invalid FEN: unknown piece "%s"'):format(ch))

            grid[rank][file] = ch
            file = file + 1
        end
    end

    assert(rank == 1 and file == 9, 'Invalid FEN: placement did not fill 8x8 board')
    return grid
end

function Board.new(fen)
    local self = setmetatable({}, Board)
    self.grid = fen_to_grid(fen)
    return self
end

function Board.get_piece(self, file, rank)
    if file < 1 or file > 8 or rank < 1 or rank > 8 then
        return nil
    end

    return self.grid[rank][file]
end

-- utility
local function file_rank_to_string(file, rank)
    local file_char = string.char(string.byte('a') + file - 1)
    return file_char .. tostring(rank)
end

-- game
Game = {}
Game.__index = Game
setmetatable(Game, GameObject)

function Game.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Game)

    local fen = 'rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1'
    local fen = '8/5k2/3p4/1p1Pp2p/pP2Pp1P/P4P1K/8/8 b - - 99 50'
    self.board = Board.new(fen)
    return self
end

function Game:render()
    -- collect all pieces
    local pieces = {}

    local function add_piece(piece_type, name, count)
        local arr = {}
        for i = 1, count do
            local piece_name = name .. tostring(i)
            local piece_entity = g_scene:find_entity_by_name(piece_name)
            local renderer = g_scene:get_mesh_renderer(piece_entity)
            renderer:set_visible(false)
            arr[#arr + 1] = piece_entity
        end
        pieces[piece_type] = arr
    end

    add_piece(WP, 'white_pawn_', 8)
    add_piece(WN, 'white_knight_', 2)
    add_piece(WB, 'white_bishop_', 2)
    add_piece(WR, 'white_rook_', 2)
    add_piece(WQ, 'white_queen_', 1)
    add_piece(WK, 'white_king_', 1)
    add_piece(BP, 'black_pawn_', 8)
    add_piece(BN, 'black_knight_', 2)
    add_piece(BB, 'black_bishop_', 2)
    add_piece(BR, 'black_rook_', 2)
    add_piece(BQ, 'black_queen_', 1)
    add_piece(BK, 'black_king_', 1)

    local offset_x = -3.5
    local offset_z = -3.5

    for rank = 1, 8 do
        for file = 1, 8 do
            local piece = self.board:get_piece(file, rank)
            if piece ~= EMPTY then
                local pool = pieces[piece]
                local piece_entity = pool[#pool]
                pool[#pool] = nil -- remove from pool
                local transform = g_scene:get_transform(piece_entity)
                local position = Vector3(rank - 1 + offset_x, 0, file - 1 + offset_z)
                transform:set_translation(position)
                local renderer = g_scene:get_mesh_renderer(piece_entity)
                renderer:set_visible(true)
            end
        end
    end
end

function Game:_process(timestep)
    self:render()
end
