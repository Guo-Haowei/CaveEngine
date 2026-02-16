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

local function coord_to_index(x, y)
    return (y - 1) * 8 + (x - 1)
end

function Game.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Game)

    self.chess = Chess.new()

    self.grid_adapter = GridAdapter.new({})
    self.grid_selector_entity = g_scene:find_entity_by_name('grid_selector')

    self.selector = GridSelector.new(self.grid_adapter, {
        can_select = function(tx, ty)
            local index = coord_to_index(tx, ty)
            local ok = self.chess:can_move(index)
            if not ok then
                logger.trace('cannot select: ' .. move.index_to_uci(index))
            end
            return ok
        end,

        can_drop = function(sx, sy, tx, ty)
            local mv = {
                from = coord_to_index(sx, sy),
                to = coord_to_index(tx, ty),
            }

            local ok = self.chess:is_legal(mv)
            if not ok then
                logger.trace('cannot drop: ' .. move.index_to_uci(mv.from) .. move.index_to_uci(mv.to))
            end
            return ok
        end,

        on_select = function(tx, ty)
            local index = coord_to_index(tx, ty)
            self.highlights = self.chess:legal_moves_from(index)
        end,

        on_commit = function(sx, sy, tx, ty)
            local mv = {
                from = coord_to_index(sx, sy),
                to = coord_to_index(tx, ty),
            }

            local ok, err = self.chess:make_move(mv)
            if ok then
                logger.trace('make move: ' .. move.index_to_uci(mv.from) .. move.index_to_uci(mv.to))
            end
            self.highlights = nil
        end,

        on_cancel = function()
            self.highlights = nil
        end,

        on_invalid = function(kind, ...)
            -- @TODO: play error sound
        end,
    })

    self.highlight_pool = {}
    for i = 1, 27 do
        local highlight = g_scene:find_entity_by_name('highlight_' .. tostring(i))
        self.highlight_pool[#self.highlight_pool + 1] = highlight
    end

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

    -- set highlights to invisible
    for i = 1, #self.highlight_pool do
        local highlight = self.highlight_pool[i]
        local renderer = g_scene:get_mesh_renderer(highlight)
        renderer:set_visible(false)
    end

    -- draw highlights
    if self.highlights ~= nil then
        for i = 1, #self.highlights do
            local mv = self.highlights[i]
            local highlight = self.highlight_pool[i]
            local transform = g_scene:get_transform(highlight)
            transform:set_translation(Vector3(math.floor(mv.to / 8), offset, mv.to % 8))
            local renderer = g_scene:get_mesh_renderer(highlight)
            renderer:set_visible(true)
        end
    end
end

function Game:_process(timestep)
    -- @TODO: refactor this part
    -- if Input.is_action_just_pressed('ui_right') == 1 then
    --     self.selector:move_focus(1, 0)
    -- end
    -- if Input.is_action_just_pressed('ui_left') == 1 then
    --     self.selector:move_focus(-1, 0)
    -- end
    -- if Input.is_action_just_pressed('ui_up') == 1 then
    --     self.selector:move_focus(0, 1)
    -- end
    -- if Input.is_action_just_pressed('ui_down') == 1 then
    --     self.selector:move_focus(0, -1)
    -- end
    -- if Input.is_action_just_pressed('ui_accept') == 1 then
    --     self.selector:confirm()
    -- end
    -- if Input.is_action_just_pressed('ui_back') == 1 then
    --     self.selector:cancel()
    -- end

    local dx = Input.get_action_strength('ui_axis_x')
    local dy = Input.get_action_strength('ui_axis_y')
    if dx > 0.5 then
        self.selector:move_focus(1, 0)
    end
    if dx < -0.5 then
        self.selector:move_focus(-1, 0)
    end
    if dy > 0.5 then
        self.selector:move_focus(0, 1)
    end
    if dy < -0.5 then
        self.selector:move_focus(0, -1)
    end

    self:render()
end