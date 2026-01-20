-- file: game.lua
local move = require('@res://scripts/chess/move.lua')
local Position = require('@res://scripts/chess/position.lua')
local Chess = require('@res://scripts/chess/chess.lua')

local GridAdapter = require('@res://scripts/grid_adapter.lua')
local GridSelector = require('@res://scripts/grid_selector.lua')

-- game
Game = {}
Game.__index = Game
setmetatable(Game, GameObject)

local function coord_to_square(x, y)
    return (y - 1) * 8 + (x - 1)
end

function Game.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Game)

    self.chess = Chess.new()

    self.grid_adapter = GridAdapter.new({})

    self.grid_selector_entity = g_scene:find_entity_by_name('grid_selector')
    self.piece_hightlight_entity = g_scene:find_entity_by_name('piece_highlight')

    self.selector = GridSelector.new(self.grid_adapter, {
        can_select = function(tx, ty)
            local piece = self.chess:get_piece(tx, ty)
            if not piece then
                return false
            end

            return self.chess:piece_color(piece) == self.chess:turn()
        end,

        can_drop = function(sx, sy, tx, ty)
            -- return chess:is_legal_move(sx, sy, tx, ty)
            return true
        end,

        on_select = function(tx, ty)
            if self.chess.get_legal_moves_from then
                -- chess.highlight_tiles = chess:get_legal_moves_from(tx, ty)
            else
                -- chess.highlight_tiles = nil
            end
        end,

        on_commit = function(sx, sy, tx, ty)
            local mv = {
                from = coord_to_square(sx, sy),
                to = coord_to_square(tx, ty),
            }
            -- Engine.log('move committed: ' .. move.index_to_square(mv.from) .. ' -> ' .. move.index_to_square(mv.to))

            local ok, err = self.chess:make_move(mv)
            if not ok then
                Engine.log('invalid move: ' .. err)
            end
            -- chess.highlight_tiles = nil
        end,

        on_cancel = function()
            -- chess.highlight_tiles = nil
        end,

        on_invalid = function(kind, ...)
            -- kind == 'select' or 'drop'
            -- you can play a sound or flash UI here
        end,
    })

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
            renderer:set_cast_shadow(false)
            arr[#arr + 1] = piece_entity
        end
        pieces[piece_type] = arr
    end

    add_piece(Position.PIECE.WP, 'white_pawn_', 8)
    add_piece(Position.PIECE.WN, 'white_knight_', 2)
    add_piece(Position.PIECE.WB, 'white_bishop_', 2)
    add_piece(Position.PIECE.WR, 'white_rook_', 2)
    add_piece(Position.PIECE.WQ, 'white_queen_', 1)
    add_piece(Position.PIECE.WK, 'white_king_', 1)
    add_piece(Position.PIECE.BP, 'black_pawn_', 8)
    add_piece(Position.PIECE.BN, 'black_knight_', 2)
    add_piece(Position.PIECE.BB, 'black_bishop_', 2)
    add_piece(Position.PIECE.BR, 'black_rook_', 2)
    add_piece(Position.PIECE.BQ, 'black_queen_', 1)
    add_piece(Position.PIECE.BK, 'black_king_', 1)

    --  place pieces on board
    for rank = 1, 8 do
        for file = 1, 8 do
            local piece = self.chess:get_piece(file, rank)
            if piece ~= nil then
                local pool = pieces[piece]
                local piece_entity = pool[#pool]
                pool[#pool] = nil -- remove from pool
                local transform = g_scene:get_transform(piece_entity)
                local position = Vector3(rank - 1, 0, file - 1)
                transform:set_translation(position)
                local renderer = g_scene:get_mesh_renderer(piece_entity)
                renderer:set_visible(true)
                renderer:set_cast_shadow(true)
            end
        end
    end

    -- place selector to focused tile
    local offset = 0.05

    if self.grid_selector_entity then
        local transform = g_scene:get_transform(self.grid_selector_entity)
        transform:set_translation(Vector3(self.selector.focus.y - 1, offset, self.selector.focus.x - 1))
    end

    if self.piece_hightlight_entity then
        local renderer = g_scene:get_mesh_renderer(self.piece_hightlight_entity)
        if self.selector.selected then
            local transform = g_scene:get_transform(self.piece_hightlight_entity)
            transform:set_translation(Vector3(self.selector.selected.y - 1, offset, self.selector.selected.x - 1))
            renderer:set_visible(true)
        else
            renderer:set_visible(false)
        end
    end
end

function Game:_process(timestep)
    -- @TODO: refactor this part
    if Input.is_action_just_pressed('ui_right') == 1 then
        self.selector:move_focus(1, 0)
    end
    if Input.is_action_just_pressed('ui_left') == 1 then
        self.selector:move_focus(-1, 0)
    end
    if Input.is_action_just_pressed('ui_up') == 1 then
        self.selector:move_focus(0, 1)
    end
    if Input.is_action_just_pressed('ui_down') == 1 then
        self.selector:move_focus(0, -1)
    end
    if Input.is_action_just_pressed('ui_accept') == 1 then
        self.selector:confirm()
    end
    if Input.is_action_just_pressed('ui_cancel') == 1 then
        self.selector:cancel()
    end

    self:render()
end