#pragma once

#define USE_REFLECTION USE_IF(!USING(PLATFORM_WASM))

#if !USING(USE_REFLECTION)

#define CAVE_META(CLASS_NAME)
#define CAVE_PROP(...)

#else

#define CAVE_META(CLASS_NAME)                             \
public:                                                   \
    friend class MetaDataTable<CLASS_NAME>;               \
    static inline constexpr ::cave::MetaTag s_meta_tag{}; \
                                                          \
private:

#define CAVE_PROP(...)

namespace cave {

struct MetaTag {
    explicit constexpr MetaTag() = default;
};

template<typename T>
concept IsReflectable = requires {
    { T::s_meta_tag } -> std::same_as<const ::cave::MetaTag&>;
};

struct FieldMetaBase;

template<typename T>
class MetaDataTable;

using MetaTableFields = std::vector<FieldMetaBase*>;

}  // namespace cave

#endif
