#pragma once
#include "cave/core/ids/Entity.h"

namespace cave {

class AssetRegistry;
class IDeserializer;
class ISerializer;
class PrefabInstanceComponent;
class Scene;

void SerializeScene(ISerializer& s, const Scene& scene, AssetRegistry* asset_reg, bool skip_prefab);
void DeserializeScene(IDeserializer& d, Scene& scene);

// @TODO: move this somewhere else
void InstantiatePrefab(Scene& scene, PrefabInstanceComponent& prefab, ecs::Entity ent);

}  // namespace cave
