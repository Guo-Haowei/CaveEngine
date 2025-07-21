#pragma once

// clang-format off
namespace cave { class Scene; }
namespace cave::jobsystem { class Context; }
// clang-format on

namespace cave {

class Scene;

void RunLightUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

void RunTransformationUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

void RunHierarchyUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

void RunAnimationUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

void RunArmatureUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

void RunObjectUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

#if 0
void RunParticleEmitterUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);

void RunMeshEmitterUpdateSystem(Scene& p_scene, jobsystem::Context& p_context, float p_timestep);
#endif

}  // namespace cave
