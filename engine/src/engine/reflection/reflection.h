#pragma once

#define USE_REFLECTION IN_USE

#define CAVE_META(CLASS_NAME)                             \
public:                                                   \
    static inline constexpr ::cave::MetaTag s_meta_tag{}; \
                                                          \
private:

#define CAVE_PROP(...)

namespace cave {

struct MetaTag {
    explicit constexpr MetaTag() = default;
};

template<typename T>
concept HasMetaTag = requires {
    { T::s_meta_tag } -> std::same_as<const ::cave::MetaTag&>;
};

}  // namespace cave
