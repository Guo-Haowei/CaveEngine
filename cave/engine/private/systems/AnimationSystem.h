#pragma once

namespace cave::jobsystem {
class Context;
}

namespace cave {

class Scene;

void RunSpriteAnimationSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunTransformAnimationSystem(Scene& scene, jobsystem::Context& ctx, float dt);

}  // namespace cave
