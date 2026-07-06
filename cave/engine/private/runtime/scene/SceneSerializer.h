#pragma once

namespace cave {

class AssetRegistry;
class IDeserializer;
class ISerializer;
class Scene;

void SerializeScene(ISerializer& s, const Scene& scene, AssetRegistry* asset_reg, bool skip_prefab);
void DeserializeScene(IDeserializer& d, Scene& scene);

std::string ToString(const Scene& scene);

}  // namespace cave
