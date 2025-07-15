#pragma once
#include <nlohmann/json.hpp>

#include "engine/math/angle.h"
#include "engine/math/box.h"
#include "engine/math/geomath.h"

namespace my {
using nlohmann::json;

class Guid;

void to_json(json& j, const Guid& p_guid);
void from_json(const json& j, Guid& p_guid);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector2f, x, y);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector3f, x, y, z);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Vector4f, x, y, z, w);

// @TODO: write to .tmp then rename, because renaming it atomic
auto Serialize(std::string_view p_path, const json& j) -> Result<void>;
auto Deserialize(std::string_view p_path, json& j) -> Result<void>;

}  // namespace my
