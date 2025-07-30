#pragma once
#include "entity.h"

namespace cave {

class Scene;

template<typename T>
struct IsComponent : std::false_type {};

template<typename T>
inline constexpr bool IsComponentV = IsComponent<T>::value;

template<typename T>
concept ComponentType = IsComponentV<std::decay_t<T>>;

}  // namespace cave

namespace cave::ecs {

class IComponentManager {
    IComponentManager(const IComponentManager&) = delete;
    IComponentManager& operator=(const IComponentManager&) = delete;

public:
    IComponentManager() = default;
    virtual ~IComponentManager() = default;
    virtual void Clear() = 0;
    virtual void Copy(const IComponentManager& p_other) = 0;
    virtual void Merge(IComponentManager& p_other) = 0;
    virtual void Remove(const Entity& p_entity) = 0;
    virtual bool Contains(const Entity& p_entity) const = 0;
    virtual size_t GetCount() const = 0;

    virtual const std::vector<Entity>& GetEntityArray() const = 0;
};

template<ComponentType T>
class ComponentManager : public IComponentManager {
public:
    ComponentManager(size_t p_capacity = 0) { Reserve(p_capacity); }

    void Reserve(size_t p_capacity);

    void Clear() override;

    void Copy(const ComponentManager<T>& p_other);

    void Copy(const IComponentManager& p_other) override;

    void Merge(ComponentManager<T>& p_other);

    void Merge(IComponentManager& p_other) override;

    void Remove(const Entity& p_entity) override;

    bool Contains(const Entity& p_entity) const override;

    T& GetComponentByIndex(size_t p_index);

    const T& GetComponentByIndex(size_t p_index) const;

    T* GetComponent(const Entity& p_entity);

    size_t GetCount() const override { return m_componentArray.size(); }

    Option<size_t> FindIndex(Entity p_entity) const {
        auto it = m_lookup.find(p_entity);
        if (it == m_lookup.end()) return None();
        return Some(it->second);
    }

    T& Create(const Entity& p_entity);

    const std::vector<Entity>& GetEntityArray() const override {
        return m_entityArray;
    }

protected:
    std::vector<T> m_componentArray;
    std::vector<Entity> m_entityArray;
    std::unordered_map<Entity, size_t> m_lookup;

    friend class ::cave::Scene;
};

class ComponentLibrary {
public:
    struct LibraryEntry {
        std::unique_ptr<IComponentManager> m_manager = nullptr;
        uint64_t m_version = 0;
    };

    template<ComponentType T>
    inline ComponentManager<T>& RegisterManager(const std::string& p_name, uint64_t p_version = 0) {
        DEV_ASSERT(m_entries.find(p_name) == m_entries.end());
        m_entries[p_name].m_manager = std::make_unique<ComponentManager<T>>();
        m_entries[p_name].m_version = p_version;
        return static_cast<ComponentManager<T>&>(*(m_entries[p_name].m_manager));
    }

private:
    std::unordered_map<std::string, LibraryEntry> m_entries;

    friend class ::cave::Scene;
};

}  // namespace cave::ecs
