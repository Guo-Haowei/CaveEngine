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

local function file_rank_to_string(file, rank)
    local file_char = string.char(string.byte('a') + file - 1)
    return file_char .. tostring(rank)
end

function Board.new(FEN)
    local self = setmetatable({}, Board)
    -- @TODO: set from FEN string
    self.grid = {
        {WR, WN, WB, WQ, WK, WB, WN, WR}, -- rank 1
        {WP, WP, WP, WP, WP, WP, WP, WP}, -- rank 2
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}, -- rank 3
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}, -- rank 4
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}, -- rank 5
        {EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY}, -- rank 6
        {BP, BP, BP, BP, BP, BP, BP, BP}, -- rank 7
        {BR, BN, BB, BQ, BK, BB, BN, BR}, -- rank 8
    }
    return self
end

function Board.get_piece(self, file, rank)
    if file < 1 or file > 8 or rank < 1 or rank > 8 then
        return nil
    end

    return self.grid[rank][file]
end

-- game
Game = {}
Game.__index = Game
setmetatable(Game, GameObject)

function Game.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Game)

    self.board = Board.new()

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

    add_piece(WP, "white_pawn_", 8)
    add_piece(WN, "white_knight_", 2)
    add_piece(WB, "white_bishop_", 2)
    add_piece(WR, "white_rook_", 2)
    add_piece(WQ, "white_queen_", 1)
    add_piece(WK, "white_king_", 1)
    add_piece(BP, "black_pawn_", 8)
    add_piece(BN, "black_knight_", 2)
    add_piece(BB, "black_bishop_", 2)
    add_piece(BR, "black_rook_", 2)
    add_piece(BQ, "black_queen_", 1)
    add_piece(BK, "black_king_", 1)

    local offset_x = -3.5
    local offset_z = -3.5

    for rank = 1, 8 do
        for file = 1, 8 do
            local piece = self.board:get_piece(file, rank)
            if piece ~= EMPTY then
                -- Engine.log_ok("piece at " .. file_rank_to_string(file, rank) .. " is " .. tostring(piece))
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
