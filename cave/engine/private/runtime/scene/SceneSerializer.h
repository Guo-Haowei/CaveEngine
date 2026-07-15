#pragma once
#include "cave/core/ids/Entity.h"

namespace cave {

class AssetRegistry;
class IDeserializer;
class ISerializer;
class PrefabInstanceComponent;
class Scene;

void SerializeScene(ISerializer& s, const Scene& scene, AssetRegistry* asset_reg);
void DeserializeScene(IDeserializer& d, Scene& scene);

void InstantiatePrefab(Scene& scene, PrefabInstanceComponent& prefab, ecs::Entity ent);

void ExportSubtree(ISerializer& s,
                   const Scene& scene,
                   ecs::Entity root,
                   AssetRegistry* asset_reg);

}  // namespace cave
