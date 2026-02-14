#pragma once

namespace cave::ecs {
class ComponentRegistry;
}  // namespace cave::ecs

namespace cave::engine {

bool InitializeCore();

void FinalizeCore();

ecs::ComponentRegistry& GetComponentRegistry();

}  // namespace cave::engine
