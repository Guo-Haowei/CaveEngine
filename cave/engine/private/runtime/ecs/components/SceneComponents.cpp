#include "SceneComponents.h"

namespace cave {

#if 0
void MeshEmitterComponent::Reset() {
    if ((int)particles.size() != maxMeshCount) {
        particles.resize(maxMeshCount);
    }

    aliveList.clear();
    aliveList.reserve(maxMeshCount);
    deadList.clear();
    deadList.reserve(maxMeshCount);
    for (int i = 0; i < maxMeshCount; ++i) {
        deadList.emplace_back(i);
    }
}

void MeshEmitterComponent::UpdateParticle(Index p_index, float p_timestep) {
    DEV_ASSERT(p_index.v < particles.size());
    auto& p = particles[p_index.v];
    DEV_ASSERT(p.lifespan >= 0.0f);

    p.scale *= (1.0f - p_timestep);
    p.scale = max(p.scale, 0.1f);
    p.velocity += p_timestep * gravity;
    p.rotation += Vector3f(p_timestep);
    p.lifespan -= p_timestep;

    p.position += p_timestep * p.velocity;
}
#endif

}  // namespace cave
