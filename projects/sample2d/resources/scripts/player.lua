-- file: player.lua
Player = {}
Player.__index = Player
setmetatable(Player, GameObject)

function Player.new(id)
    local self = GameObject.new(id)
    setmetatable(self, Player)
    Engine.log_ok('hello from player.lua')
    self.velocity = g_scene:get_velocity(self.id)
    self.transform = g_scene:get_transform(self.id)
    self.animator_id = g_scene:find_entity_by_name("player_animator_node")
    self.animator = g_scene:get_animator(self.animator_id)
    return self
end

function Player:_process(timestep)
    local move_x = Input.is_action_pressed('ui_right') - Input.is_action_pressed('ui_left')
    local jump = Input.is_action_just_pressed('ui_up')
    if jump ~= 0 then
        self.velocity.linear.y = 10
    end

    if move_x == 0 then
        self.animator:set_clip('idle')
        self.velocity.linear.x = 0
    else
        self.animator:set_clip('walk')

        -- @TODO: attach sprite as child to player
        local rotate_z = move_x < 0 and math.rad(180) or 0
        local euler = Vector3(0, rotate_z, 0)
        self.transform:set_rotation(Quaternion(euler))

        self.velocity.linear.x = move_x * 3.5
    end
end

function Player:_on_collision(other)
end