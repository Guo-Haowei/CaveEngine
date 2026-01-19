-- file: move.lua

local M = {}

local FILES = 'abcdefgh'
local RANKS = '12345678'

local function file_index(ch)
    local i = string.find(FILES, ch, 1, true)
    return i and (i - 1) or nil
end

local function rank_index(ch)
    local i = string.find(RANKS, ch, 1, true)
    return i and (i - 1) or nil
end

-- 0..63, a1 = 0, b1 = 1, ..., h1 = 7, a8 = 56
function M.square_to_index(sq)
    if type(sq) ~= 'string' or #sq ~= 2 then
        return nil, 'square must be like e4'
    end
    local f = file_index(sq:sub(1, 1))
    local r = rank_index(sq:sub(2, 2))
    if not f or not r then
        return nil, 'invalid square: ' .. sq
    end
    return r * 8 + f
end

function M.index_to_square(i)
    if type(i) ~= 'number' or i < 0 or i > 63 then
        return nil, 'index must be 0..63'
    end
    local f = i % 8
    local r = math.floor(i / 8)
    return FILES:sub(f + 1, f + 1) .. RANKS:sub(r + 1, r + 1)
end

-- Parse UCI: e2e4 or e7e8q
-- returns move table: { from=idx, to=idx, promo='q'|'r'|'b'|'n'|nil, uci=string }
function M.from_uci(uci)
    if type(uci) ~= 'string' then
        return nil, 'uci must be string'
    end
    uci = uci:lower()
    if #uci ~= 4 and #uci ~= 5 then
        return nil, 'uci must be like e2e4 or e7e8q'
    end

    local from_sq = uci:sub(1, 2)
    local to_sq = uci:sub(3, 4)
    local promo = (#uci == 5) and uci:sub(5, 5) or nil

    if promo and not (promo == 'q' or promo == 'r' or promo == 'b' or promo == 'n') then
        return nil, 'invalid promotion piece: ' .. promo
    end

    local from_i, e1 = M.square_to_index(from_sq)
    if not from_i then return nil, e1 end
    local to_i, e2 = M.square_to_index(to_sq)
    if not to_i then return nil, e2 end

    return { from = from_i, to = to_i, promo = promo, uci = uci }
end

function M.to_uci(mv)
    if type(mv) == 'string' then
        return mv:lower()
    end
    if type(mv) ~= 'table' then
        return nil, 'move must be table or string'
    end

    local from_sq, e1 = M.index_to_square(mv.from)
    if not from_sq then return nil, e1 end
    local to_sq, e2 = M.index_to_square(mv.to)
    if not to_sq then return nil, e2 end

    local s = from_sq .. to_sq
    if mv.promo then
        s = s .. mv.promo
    end
    return s
end

return M