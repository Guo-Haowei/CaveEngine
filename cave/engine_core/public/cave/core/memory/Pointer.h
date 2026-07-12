// =============================================================================
// File: cave/core/memory/Pointer.h
// =============================================================================
#pragma once
#include <memory>

namespace cave {

template<typename T, typename Deleter = std::default_delete<T>>
using Owner = std::unique_ptr<T, Deleter>;

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

template<typename T, typename... Args>
[[nodiscard]] Owner<T> MakeOwner(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
[[nodiscard]] Ref<T> MakeRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

}  // namespace cave
