#pragma once
#include "engine/systems/serialization/serialization_impl.h"
#include "engine/systems/serialization/serialization_impl.inl"

namespace my::serialize {

// @TODO: write to .tmp then rename, because renaming it atomic
auto SaveYaml(std::string_view p_path, YAML::Emitter& p_out) -> Result<void>;

auto LoadYaml(std::string_view p_path, YAML::Node& p_node) -> Result<void>;

}  // namespace my::serialize
