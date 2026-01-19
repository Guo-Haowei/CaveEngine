-- file: grid_selector.lua

local GridSelector = {}
GridSelector.__index = GridSelector

-- callbacks (all optional):
--   can_select(tx, ty) -> bool
--   can_drop(sx, sy, tx, ty) -> bool
--   on_select(tx, ty)
--   on_commit(sx, sy, tx, ty)
--   on_cancel()
--   on_invalid(kind, ...)
function GridSelector.new(grid, cb)
    local self = setmetatable({}, GridSelector)
    self.grid = grid
    self.cb = cb or {}

    self.cols, self.rows = grid:get_bounds()

    self.state = 'idle'     -- 'idle' | 'armed'
    self.selected = nil
    self.focus = { x = 1, y = 1 }
    self.hover = nil

    self.enabled = true
    return self
end

function GridSelector:set_focus(tx, ty)
    self.focus.x = math.clamp(tx, 1, self.cols)
    self.focus.y = math.clamp(ty, 1, self.rows)
end

function GridSelector:move_focus(dx, dy)
    if self.enabled then
        self:set_focus(self.focus.x + dx, self.focus.y + dy)
    end
end

function GridSelector:confirm()
    self:select_tile(self.focus.x, self.focus.y)
    print('select_tile', self.focus.x, self.focus.y)
end

function GridSelector:cancel()
    self.state = 'idle'
    self.selected = nil
    if self.cb.on_cancel then
        self.cb.on_cancel()
    end
end

function GridSelector:select_tile(tx, ty)
    if not self.enabled then
        return
    end

    if tx < 1 or tx > self.cols or ty < 1 or ty > self.rows then
        return
    end

    if self.state == 'idle' then
        if self.cb.can_select and not self.cb.can_select(tx, ty) then
            if self.cb.on_invalid then
                self.cb.on_invalid('select', tx, ty)
            end
            return
        end

        self.selected = { x = tx, y = ty }
        self.state = 'armed'

        if self.cb.on_select then
            self.cb.on_select(tx, ty)
        end

        return
    end

    -- armed: commit
    local sx, sy = self.selected.x, self.selected.y
    if self.cb.can_drop and not self.cb.can_drop(sx, sy, tx, ty) then
        if self.cb.on_invalid then
            self.cb.on_invalid('drop', sx, sy, tx, ty)
        end
        return
    end

    if self.cb.on_commit then
        self.cb.on_commit(sx, sy, tx, ty)
    end
    self:cancel()
end

function GridSelector:is_armed()
    return self.state == 'armed'
end

-- Optional mouse support later (additive)
-- function GridSelector:update_hover_from_screen(sx, sy)
--   if not (self.grid.screen_to_tile) then return end
--   local tx, ty = self.grid.screen_to_tile(sx, sy)
--   if tx then self.hover = { x = tx, y = ty } else self.hover = nil end
-- end

-- function GridSelector:click_from_screen(sx, sy)
--   if not (self.grid.screen_to_tile) then return end
--   local tx, ty = self.grid.screen_to_tile(sx, sy)
--   if tx then self:select_tile(tx, ty) end
-- end

return GridSelector