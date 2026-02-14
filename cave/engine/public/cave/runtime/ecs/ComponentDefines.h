// =============================================================================
// File: engine/public/cave/runtime/ecs/ComponentDefines.h
// =============================================================================
#pragma once
#include <type_traits>

namespace cave {

// @TODO: better ComponentType
template<typename T>
struct IsComponent : std::false_type {};

template<typename T>
inline constexpr bool IsComponentV = IsComponent<T>::value;

template<typename T>
concept ComponentType = IsComponentV<std::decay_t<T>>;

}  // namespace cave
