#pragma once

namespace cave::jobsystem {
class Context;
}

namespace cave {

class Scene;

void RunSpriteAnimationSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

}  // namespace cave
