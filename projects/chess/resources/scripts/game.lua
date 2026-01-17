-- file: game.lua

local EMPTY = 0
local WP = 1
local WN = 2
local WB = 3
local WR = 4
local WQ = 5
local WK = 6
local BP = 7
local BN = 8
local BB = 9
local BR = 10
local BQ = 11
local BK = 12

-- board
local Board = {}
Board.__index = Board

-- @TODO: set from FEN string
function Board.new()
    local self = setmetatable({}, Board)
    self.grid = {
        {WR, WN, WB, WQ, WK, WB, WN, WR},
        {WP, WP, WP, WP, WP, WP, WP, WP},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {BP, BP, BP, BP, BP, BP, BP, BP},
        {BR, BN, BB, BQ, BK, BB, BN, BR},
    }
    return self
end

function Board.get_piece(self, x, y)
    if x < 1 or x > 8 or y < 1 or y > 8 then
        return nil
    end

    return self.grid[y][x]
end

-- game
Game = {}
Game.__index = Game
setmetatable(Game, GameObject)

function Game.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Game)

    -- @TODO: get all pieces
    local wp1 = g_scene:find_entity_by_name("white_pawn_01")

    self.pieces = {}
    self.pieces[WP] = {}
    for i = 1, 8 do
        local pawn_name = "white_pawn_0" .. i
        local pawn_id = g_scene:find_entity_by_name(pawn_name)
        table.insert(self.pieces[WP], pawn_id)
    end

    print(self.pieces[WP])

    Engine.log_ok('hello from wp1' .. tostring(wp1))
    -- self.velocity = g_scene:get_velocity(self.id)
    -- self.transform = g_scene:get_transform(self.id)
    -- self.animator_id = g_scene:find_entity_by_name("player_animator_node")
    -- self.animator = g_scene:get_animator(self.animator_id)
    return self
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
