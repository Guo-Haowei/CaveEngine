#pragma once
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class CameraController final : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

private:
    void start() override;

    void update(float dt) override;

    void followTarget(float dt);

    cave::math::Vec3f ensureInBound(const cave::math::Vec3f& position);

    Entity m_target;
};

}  // namespace super_cave_boy
