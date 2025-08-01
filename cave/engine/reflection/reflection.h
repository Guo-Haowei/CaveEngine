#pragma once

// #define USE_REFLECTION USE_IF(!USING(PLATFORM_WASM))
#define USE_REFLECTION IN_USE

#if !USING(USE_REFLECTION)

#define CAVE_META(CLASS_NAME)
#define CAVE_PROP(...)

#else

#define CAVE_META(CLASS_NAME)               \
public:                                     \
    friend class MetaDataTable<CLASS_NAME>; \
    static inline constexpr ::cave::MetaTag s_meta_tag{};

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

template<typename T>
struct EnumTraits;

template<typename T>
concept HasEnumTraits = requires(T value) {
    { EnumTraits<T>::ToString(value) } -> std::same_as<std::string_view>;
    { EnumTraits<T>::FromString(std::declval<std::string_view>()) } -> std::same_as<Option<T>>;
};

#define DECLARE_ENUM_TRAITS(EnumType, ...)                                                 \
    template<>                                                                             \
    struct EnumTraits<EnumType> {                                                          \
        static constexpr std::string_view s_mappings[] = { __VA_ARGS__ };                  \
        static std::string_view ToString(EnumType p_type) {                                \
            DEV_ASSERT_INDEX(p_type, EnumType::Count);                                     \
            return s_mappings[std::to_underlying(p_type)];                                 \
        }                                                                                  \
        static Option<EnumType> FromString(std::string_view p_val) {                       \
            for (size_t i = 0; i < array_length(s_mappings); ++i) {                        \
                if (p_val == s_mappings[i]) { return Some(static_cast<EnumType>(i)); }     \
            }                                                                              \
            return None();                                                                 \
        }                                                                                  \
    };                                                                                     \
    static_assert(array_length(EnumTraits<EnumType>::s_mappings) == (int)EnumType::Count); \
    static_assert(HasEnumTraits<EnumType>)

}  // namespace cave

#endif
