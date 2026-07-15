#pragma once
#include "cave/core/containers/Containers.h"
#include "cave/runtime/script/native/NativeScript.h"

namespace super_cave_boy {

class HealthHUDController : public ::cave::NativeScript {
    using Entity = cave::ecs::Entity;

public:
    void start() override;
    void update(float dt) override;

private:
    void display(int n);

    cave::Vector<Entity> m_images;
};

}  // namespace super_cave_boy
