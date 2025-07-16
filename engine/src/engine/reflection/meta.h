#pragma once
#include "reflection.h"

namespace YAML {
class Node;
class Emitter;
}  // namespace YAML

namespace cave {

// @TODO: remove macros
#define BEGIN_REGISTRY(TYPE) ::cave::MetaDataTable<TYPE>::BeginRegistry()
#define END_REGISTRY(TYPE)   ::cave::MetaDataTable<TYPE>::EndRegistry()

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif

#define REGISTER_FIELD(TYPE, NAME, FIELD, ...) \
    ::cave::MetaDataTable<TYPE>::RegisterField(((const TYPE*)0)->FIELD, NAME, typeid(FIELD).name(), offsetof(TYPE, FIELD), ##__VA_ARGS__)

#define REGISTER_FIELD_2(TYPE, FIELD, ...) \
    ::cave::MetaDataTable<TYPE>::RegisterField(((const TYPE*)0)->FIELD, #FIELD, typeid(FIELD).name(), offsetof(TYPE, FIELD), ##__VA_ARGS__)

#define DEFINE_FILED(TYPE, DISPLAY_NAME, FIELD)                                                                    \
    std::move(std::shared_ptr<FieldMetaBase>(new FieldMeta<TYPE>(DISPLAY_NAME,                                     \
                                                                 typeid(decltype(((TYPE*)nullptr)->FIELD)).name(), \
                                                                 offsetof(TYPE, FIELD),                            \
                                                                 static_cast<FieldFlag>(0))))

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

enum FieldFlag : uint32_t;
struct SerializeYamlContext;

struct FieldMetaBase {
    const char* const name;
    const char* const type;
    const size_t offset;
    const FieldFlag flags;

    FieldMetaBase(const char* p_name,
                  const char* p_type,
                  size_t p_offset,
                  FieldFlag p_flags)
        : name(p_name), type(p_type), offset(p_offset), flags(p_flags) {
    }

    virtual ~FieldMetaBase() = default;

    template<typename T>
    const T& GetData(const void* p_object) const {
        char* ptr = (char*)p_object + offset;
        return *reinterpret_cast<T*>(ptr);
    }

    template<typename T>
    T& GetData(void* p_object) {
        char* ptr = (char*)p_object + offset;
        return *reinterpret_cast<T*>(ptr);
    }

    [[nodiscard]] virtual Result<void> DumpYaml(YAML::Emitter& p_out, const void* p_object, SerializeYamlContext& p_context) const = 0;
    [[nodiscard]] virtual Result<void> UndumpYaml(const YAML::Node& p_node, void* p_object, SerializeYamlContext& p_context) = 0;
};

template<typename T>
class FieldMeta : public FieldMetaBase {
    using FieldMetaBase::FieldMetaBase;

    Result<void> DumpYaml(YAML::Emitter& p_out, const void* p_object, SerializeYamlContext& p_context) const override;

    Result<void> UndumpYaml(const YAML::Node& p_node, void* p_object, SerializeYamlContext& p_context) override;
};

template<typename T>
class MetaDataTable {
public:
    static void BeginRegistry() {
        auto& context = GetContextInternal();
        if (!context.initialized && !context.fields.empty()) {
            CRASH_NOW_MSG("Did you call BeginRegistry() without calling EndRegistry()?");
        }

        if (context.initialized) {
            LOG_WARN("Meta table already registered! Clearing...");
            context.fields.clear();
        }
    }

    static void EndRegistry() {
        auto& context = GetContextInternal();
        context.initialized = true;
        if (context.fields.empty()) {
            LOG_WARN("No fields registered!");
        }
    }

    template<typename U>
    static void RegisterField(const U&, const char* p_name, const char* p_type, size_t p_offset, FieldFlag p_flag = FieldFlag::NONE) {
        auto& context = GetContextInternal();
        DEV_ASSERT(context.initialized == false);
#if USING(DEBUG_BUILD)
        for (const auto& field : context.fields) {
            DEV_ASSERT_MSG(field->name != p_name, std::format("field '{}' already registered, did you call it twice or accidentally registered the same fields?", p_name));
        }
#endif
        auto field = new FieldMeta<U>(p_name, p_type, p_offset, p_flag);
        context.fields.emplace_back(field);
    }

    static const MetaTableFields& GetFields() {
        if constexpr (HasMetaTag<T>) {
            return GetMetaTableFields<T>();
        } else {
            auto& context = GetContextInternal();
            DEV_ASSERT_MSG(context.initialized, "call RegisterClass() before calling GetFields()");
            return context.fields;
        }
    }

private:
    static auto& GetContextInternal() {
        static struct Context {
            bool initialized{ false };
            MetaTableFields fields{};
        } s_context;
        return s_context;
    }
};

}  // namespace cave
