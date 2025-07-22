-- file: player.lua
Player = {}
Player.__index = Player
setmetatable(Player, GameObject)

function Player.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Player)
    Engine.log_ok('hello from player.lua')
    self.transform = g_scene:get_transform(self.id)
    self.animator = g_scene:get_animator(self.id)
    return self
end

function Player:_process(timestep)
    local move_x = Input.is_action_pressed('ui_right') - Input.is_action_pressed('ui_left')
    -- local move_y = Input.is_action_pressed('ui_up') - Input.is_action_pressed('ui_down')
    if move_x == 0 then
        self.animator:set_clip('idle')
    else
        self.animator:set_clip('walk')
        local rotate_z = move_x < 0 and math.rad(180) or 0
        local euler = Vector3(0, rotate_z, 0)
        self.transform:set_rotation(Quaternion(euler))

        local translate = self.transform:get_translation()
        local speed = 4 * timestep
        translate.x = translate.x + speed * move_x
        self.transform:set_translation(translate)
    end
end

function Player:_on_collision(other)
end