#pragma once
#include "cave/core/ids/Entity.h"
#include "cave/core/ids/Guid.h"

namespace cave {

class Scene;

class PrefabExporter {
public:
    Result<Guid> exportPrefab(std::string_view path,
                              const Scene& scene,
                              ecs::Entity root);
};

}  // namespace cave
