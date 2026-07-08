// =============================================================================
// File: cave/core/containers/Containers.h
// =============================================================================
#pragma once
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace cave {

// =============================================================================
// Sequence containers
// =============================================================================

template<typename T, typename Allocator = std::allocator<T>>
using Vector = std::vector<T, Allocator>;

// =============================================================================
// Ordered associative containers
// =============================================================================

template<typename Key,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<Key>>
using Set = std::set<Key, Compare, Allocator>;

template<typename Key,
         typename Value,
         typename Compare = std::less<Key>,
         typename Allocator = std::allocator<std::pair<const Key, Value>>>
using Map = std::map<Key, Value, Compare, Allocator>;

// =============================================================================
// Unordered associative containers
// =============================================================================

template<typename Key,
         typename Hash = std::hash<Key>,
         typename Equal = std::equal_to<Key>,
         typename Allocator = std::allocator<Key>>
using HashSet = std::unordered_set<Key, Hash, Equal, Allocator>;

template<typename Key,
         typename Value,
         typename Hash = std::hash<Key>,
         typename Equal = std::equal_to<Key>,
         typename Allocator = std::allocator<std::pair<const Key, Value>>>
using HashMap = std::unordered_map<Key, Value, Hash, Equal, Allocator>;

// =============================================================================
// Strings
// =============================================================================

template<typename CharT,
         typename Traits = std::char_traits<CharT>,
         typename Allocator = std::allocator<CharT>>
using BasicString = std::basic_string<CharT, Traits, Allocator>;

using String = BasicString<char>;
using WString = BasicString<wchar_t>;

// =============================================================================
// Smart pointers
// =============================================================================

template<typename T, typename Deleter = std::default_delete<T>>
using Owner = std::unique_ptr<T, Deleter>;

template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using WeakRef = std::weak_ptr<T>;

}  // namespace cave