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

    -- data
    self.pieces = {}
    self.board = Board.new()

    -- @TODO: get all pieces
    local wp1 = g_scene:find_entity_by_name("white_pawn_01")

    local arr = {}
    for i = 1, 8 do
        local pawn_name = "white_pawn_0" .. i
        local pawn_id = g_scene:find_entity_by_name(pawn_name)
        arr[#arr + 1] = pawn_id
    end
    self.pieces[WP] = arr

    print(self.pieces[WP])

    Engine.log_ok('hello from wp1' .. tostring(wp1))
    self:render()
    return self
end

function Game:render()
    for rank = 1, 8 do
        for file = 1, 8 do
            local piece = self.board:get_piece(file, rank)
            if piece ~= EMPTY then
                Engine.log_ok("piece at " .. file_rank_to_string(file, rank) .. " is " .. tostring(piece))
            end
        end
    end
end

function Game:_process(timestep)
    for i = 1, 8 do
        local pawn_id = self.pieces[WP][i]
        local transform = g_scene:get_transform(pawn_id)
        local pos = transform:get_translation()
        pos.y = pos.y + math.sin(0.01 * 3 + i) * 0.01
        transform:set_translation(pos)
    end
    -- @TODO: move entities to desired positions
end
