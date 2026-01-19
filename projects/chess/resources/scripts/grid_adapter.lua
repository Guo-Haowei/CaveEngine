-- file: grid_adapter.lua

local GridAdapter = {}
GridAdapter.__index = GridAdapter

function GridAdapter.new(opts)
    local self = setmetatable({}, GridAdapter)

    self.cols = opts.cols or 8
    self.rows = opts.rows or 8
    return self
end

function GridAdapter:get_bounds()
    return self.cols, self.rows
end

return GridAdapter