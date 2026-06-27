#pragma once

// clang-format off
namespace cave { class Scene; }
namespace cave::jobsystem { class Context; }
// clang-format on

namespace cave {

class Scene;

void RunLightUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunTransformationUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunHierarchyUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunAnimationUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunSkeletonUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunMeshAABBUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunFacingUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

#if 0
void RunParticleEmitterUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);

void RunMeshEmitterUpdateSystem(Scene& scene, jobsystem::Context& ctx, float dt);
#endif

}  // namespace cave
