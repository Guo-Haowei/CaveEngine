-- file: player.lua
Player = {}
Player.__index = Player
setmetatable(Player, GameObject)

function Player.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Player)
    -- self.speed = Vector2(0, 0)
    -- self.displacement = Vector2(0, 0)
    Engine.log_ok('hello from player.lua')
    return self
end

function Player:OnUpdate(timestep)
    local transform = g_scene:get_transform(self.id)
    local translate = transform:get_translation()
    local speed = 10 * timestep
    local move_x = Input.is_action_pressed('ui_right') - Input.is_action_pressed('ui_left')
    local move_y = Input.is_action_pressed('ui_up') - Input.is_action_pressed('ui_down')
    translate.x = translate.x + speed * move_x
    translate.y = translate.y + speed * move_y
    transform:set_translation(translate)
end

function Player:OnCollision(other)
end